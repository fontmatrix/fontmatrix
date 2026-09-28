/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "chartwidget.h"
#include "fontmatrix_debug.h"
#include "ui_chartwidget.h"

#include "fmfontdb.h"
#include "fmglyphhighlight.h"
#include "fmuniblocks.h"
#include "fontitem.h"

#include <KLocalizedString>
#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QPrintDialog>
#include <QPrinter>
#include <QScrollBar>
#include <QStringListModel>

// Registry key used by FloatingWidgetsRegister; stable English identifier,
// never translated.
const QString ChartWidget::Name = QStringLiteral("Chart");

ChartWidget::ChartWidget(const QString &fid, const QString &block, QWidget *parent)
    : FloatingWidget(fid, Name, parent)
    , ui(new Ui::ChartWidget)
    , fontIdentifier(fid)
{
    FontItem *theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
    ui->setupUi(this);
    ui->uniLine->setEnabled(false);
    abcScene = new QGraphicsScene;
    ui->abcView->setScene(abcScene);
    ui->abcView->setRenderHint(QPainter::Antialiasing, true);
    auto cslModel(new QStringListModel);
    cslModel->setStringList(theVeryFont->getNames());
    auto cslCompleter(new QCompleter(ui->charSearchLine));
    cslCompleter->setModel(cslModel);
    ui->charSearchLine->setCompleter(cslCompleter);
    unMapGlyphName = i18nc("@item:inlistbox", "Un-Mapped Glyphs");
    allMappedGlyphName = i18nc("@item:inlistbox", "View all mapped glyphs");
    uRangeIsNotEmpty = false;
    fillUniPlanesCombo(theVeryFont);
    curGlyph = nullptr;
    fancyGlyphInUse = -1;

    if (!block.isEmpty())
        selectBlock(block);

    createConnections();
}

ChartWidget::~ChartWidget()
{
    // As we're about to delete the scene, we must tell FontItem to clear its cache
    FMFontDb::DB()->Font(fontIdentifier)->deRenderAll();
    removeConnections();
    delete ui;
    delete abcScene;
    delete curGlyph;
}

void ChartWidget::createConnections()
{
    connect(ui->abcView, &FMGlyphsView::pleaseShowSelected, this, &ChartWidget::slotShowOneGlyph);
    connect(ui->abcView, &FMGlyphsView::pleaseShowAll, this, &ChartWidget::slotShowAllGlyph);
    connect(ui->abcView, &FMGlyphsView::refit, this, &ChartWidget::slotAdjustGlyphView);
    connect(ui->abcView, &FMGlyphsView::pleaseUpdateMe, this, &ChartWidget::slotUpdateGView);
    connect(ui->abcView, &FMGlyphsView::pleaseUpdateSingle, this, &ChartWidget::slotUpdateGViewSingle);
    connect(ui->uniPlaneCombo, &QComboBox::activated, this, &ChartWidget::slotPlaneSelected);
    connect(ui->clipboardCheck, &QToolButton::toggled, this, &ChartWidget::slotShowULine);
    connect(ui->charSearchLine, &QLineEdit::returnPressed, this, &ChartWidget::slotSearchCharName);
    if (FontItem *font = FMFontDb::DB()->Font(fontIdentifier))
        connect(font, &FontItem::variationChanged, this, &ChartWidget::slotUpdateGView);

    connect(ui->toolbar, &FloatingWidgetToolBar::Close, this, &ChartWidget::close);
    connect(ui->toolbar, &FloatingWidgetToolBar::Hide, this, &ChartWidget::hide);
    connect(ui->toolbar, &FloatingWidgetToolBar::Print, this, &ChartWidget::slotPrint);
    connect(ui->toolbar, &FloatingWidgetToolBar::Detach, this, &ChartWidget::ddetach);
}

void ChartWidget::removeConnections()
{
    disconnect(ui->abcView, &FMGlyphsView::pleaseShowSelected, this, &ChartWidget::slotShowOneGlyph);
    disconnect(ui->abcView, &FMGlyphsView::pleaseShowAll, this, &ChartWidget::slotShowAllGlyph);
    disconnect(ui->abcView, &FMGlyphsView::refit, this, &ChartWidget::slotAdjustGlyphView);
    disconnect(ui->abcView, &FMGlyphsView::pleaseUpdateMe, this, &ChartWidget::slotUpdateGView);
    disconnect(ui->abcView, &FMGlyphsView::pleaseUpdateSingle, this, &ChartWidget::slotUpdateGViewSingle);
    disconnect(ui->uniPlaneCombo, &QComboBox::activated, this, &ChartWidget::slotPlaneSelected);
    disconnect(ui->clipboardCheck, &QToolButton::toggled, this, &ChartWidget::slotShowULine);
    disconnect(ui->charSearchLine, &QLineEdit::returnPressed, this, &ChartWidget::slotSearchCharName);
    if (FontItem *font = FMFontDb::DB()->Font(fontIdentifier))
        disconnect(font, &FontItem::variationChanged, this, &ChartWidget::slotUpdateGView);

    disconnect(ui->toolbar, &FloatingWidgetToolBar::Close, this, &ChartWidget::close);
    disconnect(ui->toolbar, &FloatingWidgetToolBar::Hide, this, &ChartWidget::hide);
    disconnect(ui->toolbar, &FloatingWidgetToolBar::Print, this, &ChartWidget::slotPrint);
}

void ChartWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QString ChartWidget::currentBlock()
{
    return ui->uniPlaneCombo->currentText();
}

void ChartWidget::selectBlock(const QString &uname)
{
    int idx(ui->uniPlaneCombo->findText(uname));
    if (idx < 0)
        idx = 0;
    ui->uniPlaneCombo->setCurrentIndex(idx);
    FontItem *theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
    if (theVeryFont && ui->abcView->lock()) {
        QPair<int, int> uniPair;
        if (uname == unMapGlyphName)
            uniPair = qMakePair<int, int>(-1, 100);
        else if (uname == allMappedGlyphName)
            uniPair = qMakePair<int, int>(0, 0x10FFFF);
        else
            uniPair = FMUniBlocks::interval(uname);

        int coverage = theVeryFont->countCoverage(uniPair.first, uniPair.second);
        int interval = uniPair.second - uniPair.first;
        coverage = coverage * 100 / (interval + 1); // against /0 exception

        QString statstring(i18nc("@label", "Block (%1):", QString::number(coverage) + "%"));
        ui->unicodeCoverageStat->setText(statstring);

        theVeryFont->renderAll(abcScene, uniPair.first, uniPair.second);
        ui->abcView->unlock();
    }
}

void ChartWidget::slotShowOneGlyph()
{
    //	qDebug() <<"slotShowOneGlyph()"<<abcScene->selectedItems().count();
    if (abcScene->selectedItems().isEmpty())
        return;
    if (ui->abcView->lock()) {
        QGraphicsRectItem *curGlyph = reinterpret_cast<QGraphicsRectItem *>(abcScene->selectedItems().constFirst());
        curGlyph->setSelected(false);
        if (fancyGlyphInUse < 0) {
            if (curGlyph->data(3).toInt() > 0) // Is a codepoint
            {
                fancyGlyphData = curGlyph->data(3).toInt();
                if (ui->clipboardCheck->isChecked()) {
                    new FMGlyphHighlight(abcScene, curGlyph->rect());
                    QString simpleC;
                    simpleC += QChar(fancyGlyphData);
                    QApplication::clipboard()->setText(simpleC, QClipboard::Clipboard);
                    ui->uniLine->setText(ui->uniLine->text() + simpleC);
                } else
                    fancyGlyphInUse = FMFontDb::DB()->Font(fontIdentifier)->showFancyGlyph(ui->abcView, fancyGlyphData);
            } else // Is a glyph index
            {
                fancyGlyphData = curGlyph->data(2).toInt();
                fancyGlyphInUse = FMFontDb::DB()->Font(fontIdentifier)->showFancyGlyph(ui->abcView, fancyGlyphData, true);
            }
            if (fancyGlyphInUse < 0) {
                ui->abcView->unlock();
                return;
            }
            ui->abcView->setState(FMGlyphsView::SingleView);
        }
        ui->abcView->unlock();
    } else
        qCDebug(FONTMATRIX_LOG, "cannot lock ABCview");
}

void ChartWidget::slotShowAllGlyph()
{
    // 	qDebug() <<"slotShowAllGlyph()";
    if (fancyGlyphInUse < 0)
        return;
    if (ui->abcView->lock()) {
        // 		qDebug()<<"View Locked";
        FMFontDb::DB()->Font(fontIdentifier)->hideFancyGlyph(fancyGlyphInUse);
        fancyGlyphInUse = -1;
        ui->abcView->setState(FMGlyphsView::AllView);

        ui->abcView->unlock();
    }
    // 	qDebug() <<"ENDOF slotShowAllGlyph()";
}

void ChartWidget::slotUpdateGView()
{
    FontItem *theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
    if (theVeryFont && ui->abcView->lock()) {
        QPair<int, int> uniPair;
        QString curBlockText(ui->uniPlaneCombo->currentText());
        if (curBlockText == unMapGlyphName)
            uniPair = qMakePair<int, int>(-1, 100);
        else if (curBlockText == allMappedGlyphName)
            uniPair = qMakePair<int, int>(0, 0x10FFFF);
        else
            uniPair = FMUniBlocks::interval(curBlockText);

        int coverage = theVeryFont->countCoverage(uniPair.first, uniPair.second);
        int interval = uniPair.second - uniPair.first;
        coverage = coverage * 100 / (interval + 1); // against /0 exception

        QString statstring(i18nc("@label", "Block (%1):", QString::number(coverage) + "%"));
        ui->unicodeCoverageStat->setText(statstring);

        theVeryFont->renderAll(abcScene, uniPair.first, uniPair.second);
        ui->abcView->unlock();
    }
}

void ChartWidget::slotAdjustGlyphView([[maybe_unused]] int width)
{
    //	if ( !theVeryFont )
    //		return;

    // 	theVeryFont->adjustGlyphsPerRow ( width );
    //	slotView ( true );
}

void ChartWidget::slotUpdateGViewSingle()
{
    FontItem *theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
    if (theVeryFont && ui->abcView->lock()) {
        // 			qDebug() <<"1.FGI"<<fancyGlyphInUse;
        theVeryFont->hideFancyGlyph(fancyGlyphInUse);
        if (fancyGlyphData > 0) // Is a codepoint
        {
            fancyGlyphInUse = theVeryFont->showFancyGlyph(ui->abcView, fancyGlyphData);
            // 				qDebug() <<"2.FGI"<<fancyGlyphInUse;
        } else // Is a glyph index
        {
            fancyGlyphInUse = theVeryFont->showFancyGlyph(ui->abcView, fancyGlyphData, true);
            // 				qDebug() <<"3.FGI"<<fancyGlyphInUse;
        }
        ui->abcView->unlock();
    }
}

void ChartWidget::slotPlaneSelected(int i)
{
    //	qDebug()<<"slotPlaneSelected"<<i<<uniPlaneCombo->currentIndex();
    FontItem *theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
    if (i != ui->uniPlaneCombo->currentIndex())
        ui->uniPlaneCombo->setCurrentIndex(i);

    bool stickState = uRangeIsNotEmpty;
    uRangeIsNotEmpty = true;
    slotShowAllGlyph();
    slotUpdateGView();
    if ((stickState == false) && theVeryFont) {
        fillUniPlanesCombo(theVeryFont);
    }
    ui->abcView->verticalScrollBar()->setValue(0);
}

void ChartWidget::slotShowULine(bool checked)
{
    if (checked) {
        ui->uniLine->setText("");
        ui->uniLine->setEnabled(true);
    } else {
        ui->uniLine->setEnabled(false);
    }
}

void ChartWidget::slotSearchCharName()
{
    FontItem *theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
    if (!theVeryFont)
        return;
    QString name(ui->charSearchLine->text());
    unsigned short cc(0);
    bool searchCodepoint(false);
    if (name.startsWith("U+") || name.startsWith("u+") || name.startsWith("+")) {
        QString vString(name.mid(name.indexOf("+")));
        bool ok(false);
        cc = vString.toInt(&ok, 16);
        if (!ok)
            cc = 0;
        searchCodepoint = true;
    } else
        cc = theVeryFont->getNamedChar(name);
    // 	qDebug()<<"CS"<<name<<cc;
    if (!cc) {
        // TODO display a usefull message
        // 		charSearchLine->clear();
        return;
    }

    for (const auto blocksList = FMUniBlocks::blocks(); const auto &key : blocksList) {
        QPair<int, int> p(FMUniBlocks::interval(key));
        if ((cc >= p.first) && (cc <= p.second)) {
            int idx(ui->uniPlaneCombo->findText(key));
            slotPlaneSelected(idx);
            int sv(0);
            bool first(true);
            do {
                if (first)
                    first = false;
                else {
                    ui->abcView->verticalScrollBar()->setValue(sv + ui->abcView->height());
                    sv = ui->abcView->verticalScrollBar()->value();
                }
                for (const auto itemsList = abcScene->items(); auto *sit : itemsList) {
                    if ((sit->data(1).toString() == "select") && (sit->data(3).toInt() == cc)) {
                        auto ms(reinterpret_cast<QGraphicsRectItem *>(sit));
                        if (ms) {
                            QRectF rf(ms->rect());
                            new FMGlyphHighlight(abcScene, rf, 2000, 160);
                        } else
                            qCWarning(FONTMATRIX_LOG) << "ERROR: An select item not being a QRect?";
                        return;
                    }
                }
            } while (sv < ui->abcView->verticalScrollBar()->maximum());
            return;
        }
    }

    // if user was looking for a name and we did not find it in
    // Unicode blocks, it must be unmapped.
    if (!searchCodepoint) {
        int idx(ui->uniPlaneCombo->findText(unMapGlyphName));
        slotPlaneSelected(idx);
        int sv(0);
        bool first(true);
        do {
            if (first)
                first = false;
            else {
                ui->abcView->verticalScrollBar()->setValue(sv + ui->abcView->height());
                sv = ui->abcView->verticalScrollBar()->value();
            }
            for (const auto loopItems = abcScene->items(); auto *sit : loopItems) {
                if ((sit->data(1).toString() == "select") && (sit->data(3).toInt() == cc)) {
                    auto ms(reinterpret_cast<QGraphicsRectItem *>(sit));
                    if (ms) {
                        QRectF rf(ms->rect());
                        new FMGlyphHighlight(abcScene, rf, 2000, 160);
                    } else
                        qCWarning(FONTMATRIX_LOG) << "ERROR: An select item not being a QRect?";
                    return;
                }
            }
        } while (sv < ui->abcView->verticalScrollBar()->maximum());
        return;
    }
}

void ChartWidget::fillUniPlanesCombo(FontItem *item)
{
    QString stickyRange(ui->uniPlaneCombo->currentText());
    // 	qDebug()<<"STiCKyRaNGe :: "<<stickyRange;
    int stickyIndex(0);

    ui->uniPlaneCombo->clear();

    int begin(0);
    int end(0);
    QString lastBlock(FMUniBlocks::lastBlock(begin, end));
    QString block(FMUniBlocks::firstBlock(begin, end));
    bool first(true);
    do {
        if (first)
            first = false;
        else
            block = FMUniBlocks::nextBlock(begin, end);

        int codecount(item->countCoverage(begin, end));
        if (codecount > 0) {
            // 			qDebug() << p << codecount;
            ui->uniPlaneCombo->addItem(block);
            if (block == stickyRange) {
                stickyIndex = ui->uniPlaneCombo->count() - 1;
                uRangeIsNotEmpty = true;
            }
        } else {
            if (block == stickyRange) {
                stickyIndex = ui->uniPlaneCombo->count() - 1;
                uRangeIsNotEmpty = true;
            }
        }

    } while (lastBlock != block);
    if (item->countCoverage(-1, 100) > 0) {
        ui->uniPlaneCombo->addItem(unMapGlyphName);
        if (unMapGlyphName == stickyRange) {
            stickyIndex = ui->uniPlaneCombo->count() - 1;
            uRangeIsNotEmpty = true;
        }
    }
    ui->uniPlaneCombo->addItem(allMappedGlyphName);
    if (allMappedGlyphName == stickyRange) {
        stickyIndex = ui->uniPlaneCombo->count() - 1;
        uRangeIsNotEmpty = true;
    }

    ui->uniPlaneCombo->setCurrentIndex(stickyIndex);
}

void ChartWidget::slotPrint()
{
    FontItem *font(FMFontDb::DB()->Font(fontIdentifier));
    if (font == nullptr)
        return;

    if (printer == nullptr)
        printer = new QPrinter(QPrinter::HighResolution);
    if (printDialog == nullptr) {
        printDialog = new QPrintDialog(printer, this);
        connect(printDialog, qOverload<QPrinter *>(&QPrintDialog::accepted), this, &ChartWidget::slotDoPrinting);
    }

    printDialog->setWindowTitle("Fontmatrix - " + i18nc("@title:window", "Print Chart") + " - " + font->fancyName());

    printDialog->open();
}

void ChartWidget::slotDoPrinting()
{
    FontItem *font(FMFontDb::DB()->Font(fontIdentifier));
    printer->setFullPage(true);
    QPainter aPainter(printer);

    double pWidth(printer->pageRect(QPrinter::DevicePixel).width());
    double pHeight(printer->pageRect(QPrinter::DevicePixel).height());
    double pFactor(printer->resolution());

    qCDebug(FONTMATRIX_LOG) << "Paper :" << pWidth << pHeight;
    qCDebug(FONTMATRIX_LOG) << "Resolution :" << pFactor;
    qCDebug(FONTMATRIX_LOG) << "P/R*72:" << pWidth / pFactor * 72.0 << pHeight / pFactor * 72.0;

    QRectF targetR(pWidth * 0.1, pHeight * 0.1, pWidth * 0.8, pHeight * 0.8);

    QRectF sourceR(0, 0, pWidth / pFactor * 72.0, pHeight / pFactor * 72.0);
    QGraphicsScene pScene(sourceR);

    int maxCharcode(0x10FFFF);
    int beginCharcode(0);
    int numP(0);
    bool first(true);
    while (beginCharcode < maxCharcode) {
        qCDebug(FONTMATRIX_LOG) << "Chart(" << ++numP << ") ->" << beginCharcode << maxCharcode;
        QList<QGraphicsItem *> lgit(pScene.items());
        for (auto *git : std::as_const(lgit)) {
            pScene.removeItem(git);
            delete git;
        }

        int stopAtCode(font->renderChart(&pScene, beginCharcode, maxCharcode, sourceR.width(), sourceR.height()));
        qCDebug(FONTMATRIX_LOG) << "Control" << beginCharcode << stopAtCode;

        if (stopAtCode == beginCharcode)
            break;

        if (first) {
            first = false;
        } else {
            printer->newPage();
        }
        aPainter.drawText(targetR.bottomLeft(),
                          font->fancyName() + "[U" + QString::number(beginCharcode, 16).toUpper() + ", U" + QString::number(stopAtCode, 16).toUpper() + "]");
        pScene.render(&aPainter, targetR, sourceR, Qt::KeepAspectRatio);

        beginCharcode = stopAtCode;
    }
}

#include "moc_chartwidget.cpp"
