/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "samplewidget.h"
#include "fmbaseshaper.h"
#include "fmconfig.h"
#include "fmfontdb.h"
#include "fmlayout.h"
#include "fmvariationswidget.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "opentypetags.h"
#include "sampletoolbar.h"
#include "textprogression.h"
#include "typotek.h"
#include "ui_samplewidget.h"

#include <KLocalizedString>
#include <QApplication>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileSystemWatcher>
#include <QKeyEvent>
#include <QMap>
#include <QPrintDialog>
#include <QPrinter>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QTreeWidgetItem>

QByteArray SampleWidget::State::toByteArray() const
{
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds << sampleName;
    ds << fontSize;
    ds << renderHinting;
    ds << shaper;
    ds << script;
    return b;
}

void FMLayoutThread::setLayout(FMLayout *l, const QList<GlyphList> &spec, double fs, FontItem *f, unsigned int hinting)
{
    pLayout = l;
    pLayout->setContext(false);
    gl = spec;
    fontSize = fs;
    font = f;
    fHinting = hinting;
}

void FMLayoutThread::run()
{
    auto tf(new FontItem(font->path(), font->family(), font->variant(), font->type(), font->isActivated()));
    tf->setFTHintMode(fHinting);
    pLayout->doLayout(gl, fontSize, tf);
    delete tf;
}

SampleWidget::State SampleWidget::State::fromByteArray(QByteArray b)
{
    QDataStream ds(&b, QIODevice::ReadOnly);
    QString sn;
    double fs;
    unsigned int rh;
    QString sh;
    QString sc;
    ds >> sn;
    ds >> fs;
    ds >> rh;
    ds >> sh;
    ds >> sc;
    //	State*  pState(new State(sn,fs,rh,sh,sc));
    //	State rState(*pState);
    sampleName = sn;
    fontSize = fs;
    renderHinting = rh;
    shaper = sh;
    script = sc;
    // A copy used to come out as "set" whatever the original said, and
    // restoring the saved state relied on it. Say it here instead.
    set = true;
    //	return State(sn,fs,rh,sh,sc);
    return *this;
}

// Registry key used by FloatingWidgetsRegister; stable English identifier,
// never translated.
const QString SampleWidget::Name = QStringLiteral("Sample");

SampleWidget::SampleWidget(const QString &fid, QWidget *parent)
    : FloatingWidget(fid, Name, parent)
    , ui(new Ui::SampleWidget)
    , fontIdentifier(fid)
{
    layoutTimer = new QTimer(this);
    layoutTimer->setSingleShot(true);
    layoutWait = 150;
    layoutForPrint = false;
    layoutSwitch = false;
    ui->setupUi(this);
    //	ui->textProgression->setVisible(false);

    sampleToolBar = new SampleToolBar(this);
    ui->sampleGridLayout->addWidget(sampleToolBar, 1, 0, Qt::AlignRight | Qt::AlignBottom);

    // the axes of a variable font, on the page the panel is taken from while it is shown
    variationsWidget = new FMVariationsWidget(ui->stackedViews->widget(VIEW_PAGE_VARIATIONS));
    variationsWidget->setFont(FMFontDb::DB()->Font(fid));
    sampleToolBar->showButton(SampleToolBar::VariationsButton, variationsWidget->font() != nullptr);

    sampleNameEditor = new QStyledItemDelegate(ui->sampleTextTree);
    ui->sampleTextTree->setItemDelegate(sampleNameEditor);
    refillSampleList();
    fillOTTree();

#ifdef PLATFORM_APPLE
    fileInfo.setFile(fid);
    fileLastModified = fileInfo.lastModified().toMSecsSinceEpoch();
#endif
    sysWatcher = new QFileSystemWatcher(this);
    sysWatcher->addPath(fid);
    reloadTimer = new QTimer(this);
    reloadTimer->setInterval(1000);

    loremScene = new QGraphicsScene;
    ftScene = new QGraphicsScene;
    QRectF pageRect(0, 0, 597.6, 842.4); // TODO find means to smartly decide of page size (here, iso A4)

    loremScene->setSceneRect(pageRect);

    ftScene->setSceneRect(0, 0, 597.6 * typotek::getInstance()->getDpiX() / 72.0, 842.4 * typotek::getInstance()->getDpiX() / 72.0);
    ui->loremView->setScene(loremScene);
    ui->loremView->locker = false;
    double horiScaleT(typotek::getInstance()->getDpiX() / 72.0);
    double vertScaleT(typotek::getInstance()->getDpiY() / 72.0);
    QTransform adjustAbsoluteViewT(horiScaleT, 0, 0, vertScaleT, 0, 0);
    ui->loremView->setTransform(adjustAbsoluteViewT, false);

    ui->loremView_FT->setScene(ftScene);
    ui->loremView_FT->locker = false;
    ui->loremView_FT->fakePage();

    layoutThread = new FMLayoutThread;

    FontItem *cf(FMFontDb::DB()->Font(fid));
    textLayoutVect = new FMLayout(loremScene, cf);
    textLayoutFT = new FMLayout(ftScene);

    State s;
    s.fontSize = 14;
    QByteArray bs = FMConfig::value(QStringLiteral("Sample/state"), s.toByteArray()).toByteArray();
    setState(s.fromByteArray(bs));
    sampleRatio = 1.2;
    sampleInterSize = sampleFontSize * sampleRatio;

    createConnections();
    slotView();
}

SampleWidget::~SampleWidget()
{
    removeConnections();

    // Disconnect all signals from layoutThread and textLayoutFT before waiting,
    // so that layoutFinished() / finished() cannot invoke slotView() on a
    // partially destroyed object.
    layoutThread->disconnect();
    textLayoutFT->disconnect();

    // Ask the running layout to stop and wait for the thread to exit cleanly.
    // textLayoutFT->stopLayout() sets stopIt=true; the layout loop checks it.
    textLayoutFT->stopLayout();
    layoutThread->wait();
    delete layoutThread;

    delete ui;
    delete loremScene;
    delete ftScene;
    delete textLayoutFT;
    delete textLayoutVect;
}

void SampleWidget::createConnections()
{
    // connections

    connect(ui->loremView, &FMSampleTextView::pleaseUpdateMe, this, &SampleWidget::slotUpdateSView);
    connect(ui->loremView, &FMSampleTextView::pleaseZoom, this, &SampleWidget::slotZoom);

    connect(ui->loremView_FT, &FMSampleTextView::pleaseZoom, this, &SampleWidget::slotZoom);
    connect(ui->loremView_FT, &FMSampleTextView::pleaseUpdateMe, this, &SampleWidget::slotUpdateRView);

    connect(textLayoutVect, &FMLayout::updateLayout, this, &SampleWidget::slotView);
    connect(this, &SampleWidget::stopLayout, textLayoutVect, &FMLayout::stopLayout);
    connect(textLayoutFT, &FMLayout::updateLayout, this, &SampleWidget::slotView);
    connect(this, &SampleWidget::stopLayout, textLayoutFT, &FMLayout::stopLayout);

    connect(ui->sampleTextTree, &QTreeWidget::itemSelectionChanged, this, &SampleWidget::slotSampleChanged);
    connect(ui->sampleTextTree, &QTreeWidget::itemSelectionChanged, this, &SampleWidget::slotEditSample);
    connect(sampleToolBar, &SampleToolBar::SizeChanged, this, &SampleWidget::slotLiveFontSize);

    connect(ui->OpenTypeTree, &QTreeWidget::itemClicked, this, &SampleWidget::slotFeatureChanged);
    connect(ui->saveDefOTFBut, &QPushButton::released, this, &SampleWidget::slotDefaultOTF);
    connect(ui->resetDefOTFBut, &QPushButton::released, this, &SampleWidget::slotResetOTF);

    //	connect ( ui->textProgression, SIGNAL ( stateChanged (  ) ),this ,SLOT(slotProgressionChanged()));

    connect(ui->toolbar, &FloatingWidgetToolBar::Print, this, &SampleWidget::slotPrint);
    connect(ui->toolbar, &FloatingWidgetToolBar::Close, this, &SampleWidget::close);
    connect(ui->toolbar, &FloatingWidgetToolBar::Hide, this, &SampleWidget::hide);
    connect(ui->toolbar, &FloatingWidgetToolBar::Detach, this, &SampleWidget::ddetach);

    connect(sysWatcher, &QFileSystemWatcher::fileChanged, this, &SampleWidget::slotFileChanged);
    connect(reloadTimer, &QTimer::timeout, this, &SampleWidget::slotReload);
    connect(layoutTimer, &QTimer::timeout, this, &SampleWidget::doRender);

    connect(this, &SampleWidget::stateChanged, this, &SampleWidget::saveState);

    connect(sampleToolBar, &SampleToolBar::OpenTypeToggled, this, &SampleWidget::slotShowOpenType);
    connect(sampleToolBar, &SampleToolBar::VariationsToggled, this, &SampleWidget::slotShowVariations);
    connect(variationsWidget, &FMVariationsWidget::coordinatesChanged, this, &SampleWidget::slotView);
    connect(sampleToolBar, &SampleToolBar::SampleToggled, this, &SampleWidget::slotShowSamples);
    connect(sampleToolBar, &SampleToolBar::ScriptSelected, this, &SampleWidget::slotScriptChange);

    connect(ui->addSampleButton, &QPushButton::clicked, this, &SampleWidget::slotAddSample);
    connect(ui->removeSampleButton, &QPushButton::clicked, this, &SampleWidget::slotRemoveSample);
    connect(sampleNameEditor, &QStyledItemDelegate::closeEditor, this, &SampleWidget::slotSampleNameEdited);
    connect(ui->sampleEdit, &QPlainTextEdit::textChanged, this, &SampleWidget::slotUpdateSample);

    connect(textLayoutFT, &FMLayout::clearScene, this, &SampleWidget::clearFTScene);
}

void SampleWidget::removeConnections()
{
    disconnect(ui->loremView, &FMSampleTextView::pleaseUpdateMe, this, &SampleWidget::slotUpdateSView);
    disconnect(ui->loremView, &FMSampleTextView::pleaseZoom, this, &SampleWidget::slotZoom);

    disconnect(ui->loremView_FT, &FMSampleTextView::pleaseZoom, this, &SampleWidget::slotZoom);
    disconnect(ui->loremView_FT, &FMSampleTextView::pleaseUpdateMe, this, &SampleWidget::slotUpdateRView);

    disconnect(textLayoutVect, &FMLayout::updateLayout, this, &SampleWidget::slotView);
    disconnect(this, &SampleWidget::stopLayout, textLayoutVect, &FMLayout::stopLayout);
    disconnect(textLayoutFT, &FMLayout::updateLayout, this, &SampleWidget::slotView);
    disconnect(this, &SampleWidget::stopLayout, textLayoutFT, &FMLayout::stopLayout);

    disconnect(ui->sampleTextTree, &QTreeWidget::itemSelectionChanged, this, &SampleWidget::slotSampleChanged);
    disconnect(sampleToolBar, &SampleToolBar::SizeChanged, this, &SampleWidget::slotLiveFontSize);

    disconnect(ui->OpenTypeTree, &QTreeWidget::itemClicked, this, &SampleWidget::slotFeatureChanged);
    disconnect(ui->saveDefOTFBut, &QPushButton::released, this, &SampleWidget::slotDefaultOTF);
    disconnect(ui->resetDefOTFBut, &QPushButton::released, this, &SampleWidget::slotResetOTF);

    //	disconnect ( ui->textProgression, SIGNAL ( stateChanged (  ) ),this ,SLOT(slotProgressionChanged()));

    disconnect(ui->toolbar, &FloatingWidgetToolBar::Print, this, &SampleWidget::slotPrint);
    disconnect(ui->toolbar, &FloatingWidgetToolBar::Close, this, &SampleWidget::close);
    disconnect(ui->toolbar, &FloatingWidgetToolBar::Hide, this, &SampleWidget::hide);
    disconnect(ui->toolbar, &FloatingWidgetToolBar::Detach, this, &SampleWidget::ddetach);

    disconnect(sysWatcher, &QFileSystemWatcher::fileChanged, this, &SampleWidget::slotFileChanged);
    disconnect(layoutTimer, &QTimer::timeout, this, &SampleWidget::doRender);

    disconnect(this, &SampleWidget::stateChanged, this, &SampleWidget::saveState);
}

void SampleWidget::changeEvent(QEvent *e)
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

QGraphicsScene *SampleWidget::textScene() const
{
    return loremScene;
}

SampleWidget::State SampleWidget::state() const
{
    State ret;
    ret.set = true;
    ret.fontSize = sampleToolBar->getFontSize();
    //	ret.renderRaster = ui->freetypeRadio->isChecked();
    ret.renderHinting = 0;
    //	if(ui->normalHinting->isChecked())
    //		ret.renderHinting = 1;
    //	else if(ui->lightHinting->isChecked())
    //		ret.renderHinting = 2;
    ret.sampleName = ui->sampleTextTree->currentItem()->data(0, Qt::UserRole).toString();
    if (ui->useShaperCheck->isChecked()) {
        ret.script = ui->langCombo->currentText();
        ret.shaper = ui->shaperTypeCombo->currentText();
    }
    return ret;
}

void SampleWidget::setState(const SampleWidget::State &s)
{
    if (!s.set)
        return;
    sampleToolBar->setFontSize(s.fontSize);
    reSize(s.fontSize, s.fontSize * sampleRatio);

    //	{
    //		switch(s.renderHinting)
    //		{
    //		case 0: ui->noHinting->setChecked(true);
    //			break;
    //		case 1: ui->normalHinting->setChecked(true);
    //			break;
    //		case 2: ui->lightHinting->setChecked(true);
    //			break;
    //		default:break;
    //		}
    //	}

    // A new profile names no sample. Without this the first of the tree wins,
    // whatever its script is.
    const QString wanted(s.sampleName.isEmpty() ? typotek::getInstance()->defaultSampleName() : s.sampleName);
    QTreeWidgetItem *targetItem = nullptr;
    for (int i(0); i < ui->sampleTextTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *tli(ui->sampleTextTree->topLevelItem(i));
        for (int ii(0); ii < tli->childCount(); ++ii) {
            if (tli->child(ii)->data(0, Qt::UserRole).toString() == wanted) {
                targetItem = tli->child(ii);
                typotek::getInstance()->namedSample(wanted);
                break;
            }
        }
        if (targetItem != nullptr)
            break;
    }
    //	qDebug()<<"TI"<<targetItem;
    if (targetItem != nullptr)
        ui->sampleTextTree->setCurrentItem(targetItem, 0, QItemSelectionModel::SelectCurrent);

    //	if(!s.shaper.isEmpty())
    //	{
    //		ui->useShaperCheck->setChecked(true);
    //		ui->shaperTypeCombo->setCurrentIndex(ui->shaperTypeCombo->findText(s.shaper));
    //		ui->langCombo->setCurrentIndex(ui->langCombo->findText(s.script));
    //	}

    slotView();
    slotEditSample();
}

void SampleWidget::slotView()
{
    if (layoutForPrint) {
        doRender();
        return;
    }
    layoutTimer->start(layoutWait);
}

void SampleWidget::doRender()
{
    FontItem *f(FMFontDb::DB()->Font(fontIdentifier));
    if (!f)
        return;

    FMLayout *textLayout = layoutForPrint ? textLayoutVect : textLayoutFT;

    bool wantDeviceDependant = !layoutForPrint;
    f->setFTHintMode(hinting());
    f->setProgression(PROGRESSION_LTR);
    f->setFTRaster(wantDeviceDependant);

    textLayout->setContext(true);
    textLayout->setDeviceIndy(!wantDeviceDependant);
    textLayout->setAdjustedSampleInter(sampleInterSize);

    double fSize(sampleFontSize);
    bool processFeatures = f->isOpenType() && !deFillOTTree().isEmpty();
    QString script(sampleToolBar->getScript());
    bool processScript(!script.isEmpty());

    QList<GlyphList> list;
    QStringList stl(typotek::getInstance()->namedSample().split("\n"));
    if (processScript) {
        for (int p(0); p < stl.count(); ++p)
            list << f->glyphs(stl[p], fSize, script);
    } else if (processFeatures) {
        for (int p(0); p < stl.count(); ++p)
            list << f->glyphs(stl[p], fSize, deFillOTTree());
    } else if (f->isOpenType()) {
        // Nothing picked: what an application would show, kerning, ligatures
        // and the shaping of the script included
        for (int p(0); p < stl.count(); ++p)
            list << f->glyphsShaped(stl[p], fSize);
    } else {
        for (int p(0); p < stl.count(); ++p)
            list << f->glyphs(stl[p], fSize);
    }

    auto tf = new FontItem(f->path(), f->family(), f->variant(), f->type(), f->isActivated());
    tf->setFTHintMode(hinting());
    if (variationsWidget->font())
        tf->setVariationCoordinates(f->variationCoordinates());
    textLayout->doLayout(list, fSize, tf);
    delete tf;

    if (!layoutForPrint)
        endLayout();
}

void SampleWidget::drawPixmap(int index, double fontsize, double x, double y)
{
    if (index < 0) {
        disconnect(textLayoutFT, &FMLayout::drawPixmapForMe, this, &SampleWidget::drawPixmap);
        return;
    }
    FontItem *f(FMFontDb::DB()->Font(fontIdentifier));
    if (!f)
        return;
    ++pixmapDrawn;
    QGraphicsPixmapItem *glyph = f->itemFromGindexPix(index, fontsize);
    if (!glyph)
        return;
    ftScene->addItem(glyph);
    glyph->setZValue(100.0);
    glyph->setPos(x, y);
}

void SampleWidget::drawBaseline(double y)
{
    QGraphicsLineItem *l = ftScene->addLine(0, y, ftScene->width(), y);
    l->setData(GLYPH_DATA_GLYPH, 1);
}

void SampleWidget::clearFTScene()
{
    qCDebug(FONTMATRIX_LOG) << "SampleWidget::clearFTScene" << layoutSwitch;
    //	if(layoutSwitch)
    //		return;
    for (const auto itemsList = ftScene->items(); auto *gi : itemsList) {
        if (gi->data(GLYPH_DATA_GLYPH).toInt() > 0)
            delete gi;
    }
}

void SampleWidget::endLayout()
{
    QPointF texttopLeft(ui->loremView_FT->mapFromScene(textLayoutFT->getRect().topLeft()));
    ui->loremView_FT->translate(10 - texttopLeft.x(), 10 - texttopLeft.y());
    ui->loremView_FT->update();
    //	qDebug()<<"Pixmaps:"<<pixmapDrawn;
}

void SampleWidget::fillOTTree()
{
    ui->OpenTypeTree->clear();
    ui->langCombo->clear();
    ui->langCombo->setEnabled(false);
    ui->useShaperCheck->setCheckState(Qt::Unchecked);
    ui->useShaperCheck->setEnabled(false);
    typotek *typo(typotek::getInstance());
    QStringList scripts;
    FontItem *theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
    if (theVeryFont && theVeryFont->isOpenType()) {
        FMOtf *otf = theVeryFont->takeOTFInstance();
        for (const auto tablesList = otf->get_tables(); const auto &table : tablesList) {
            otf->set_table(table);
            auto tab_item = new QTreeWidgetItem(ui->OpenTypeTree, QStringList(table));
            tab_item->setExpanded(true);
            for (const auto scriptsList = otf->get_scripts(); const auto &script : scriptsList) {
                scripts << script;
                otf->set_script(script);
                auto script_item = new QTreeWidgetItem(tab_item, QStringList(script));
                script_item->setExpanded(true);
                for (const auto langsList = otf->get_langs(); const auto &lang : langsList) {
                    otf->set_lang(lang);
                    auto lang_item = new QTreeWidgetItem(script_item, QStringList(lang));
                    lang_item->setExpanded(true);
                    for (const auto featuresList = otf->get_features(); const auto &feature : featuresList) {
                        QStringList f(feature);
                        f << OTTagMeans(feature);
                        auto feature_item = new QTreeWidgetItem(lang_item, f);
                        feature_item->setCheckState(0, Qt::Unchecked);
                        if (table == "GPOS") {
                            if (typo->getDefaultOTFScript() == script && typo->getDefaultOTFLang() == lang && typo->getDefaultOTFGPOS().contains(feature)) {
                                feature_item->setCheckState(0, Qt::Checked);
                            }
                        } else if (table == "GSUB") {
                            if (typo->getDefaultOTFScript() == script && typo->getDefaultOTFLang() == lang && typo->getDefaultOTFGSUB().contains(feature)) {
                                feature_item->setCheckState(0, Qt::Checked);
                            }
                        }
                    }
                }
            }
        }
        ui->OpenTypeTree->resizeColumnToContents(0);
        theVeryFont->releaseOTFInstance(otf);
    }
    scripts = QStringList(QSet<QString>(scripts.begin(), scripts.end()).values());
    // 	scripts.removeAll ( "latn" );
    //	if ( !scripts.isEmpty() )
    {
        //		ui->langCombo->setEnabled ( true );
        //		ui->useShaperCheck->setEnabled ( true );
        //		ui->langCombo->addItems ( scripts );
        sampleToolBar->setScripts(scripts);
    }
}

OTFSet SampleWidget::deFillOTTree()
{
    // 	qDebug() << "MainViewWidget::deFillOTTree()";
    OTFSet ret;
    // 	qDebug() << ui->OpenTypeTree->topLevelItemCount();
    for (int table_index = 0; table_index < ui->OpenTypeTree->topLevelItemCount(); ++table_index) // tables
    {
        // 		qDebug() << "table_index = " << table_index;
        QTreeWidgetItem *table_item = ui->OpenTypeTree->topLevelItem(table_index);
        // 		qDebug() <<  table_item->text(0);
        for (int script_index = 0; script_index < table_item->childCount(); ++script_index) // scripts
        {
            QTreeWidgetItem *script_item = table_item->child(script_index);
            // 			qDebug() << "\tscript_index = " <<  script_index << script_item->text(0);
            for (int lang_index = 0; lang_index < script_item->childCount(); ++lang_index) // langs
            {
                QTreeWidgetItem *lang_item = script_item->child(lang_index);
                // 				qDebug() << "\t\tlang_index = "<< lang_index << lang_item->text(0);
                for (int feature_index = 0; feature_index < lang_item->childCount(); ++feature_index) // features
                {
                    // 					qDebug() << lang_item->childCount() <<" / "<<  feature_index;
                    QTreeWidgetItem *feature_item = lang_item->child(feature_index);
                    // 					qDebug() << "\t\t\tfeature_item -> "<< feature_item->text(0);
                    if (feature_item->checkState(0) == Qt::Checked) {
                        if (table_item->text(0) == "GPOS") {
                            ret.script = script_item->text(0);
                            ret.lang = lang_item->text(0);
                            ret.gpos_features.append(feature_item->text(0));
                        }
                        if (table_item->text(0) == "GSUB") {
                            ret.script = script_item->text(0);
                            ret.lang = lang_item->text(0);
                            ret.gsub_features.append(feature_item->text(0));
                        }
                    }
                }
            }
        }
    }
    // 	qDebug() << "endOf";
    return ret;
}

// void SampleWidget::slotChangeViewPage(QAbstractButton* but)
//{
//	QString radioName( but->objectName() );

//	if(radioName == "freetypeRadio" )
//	{
//		ui->stackedViews->setCurrentIndex(VIEW_PAGE_FREETYPE);
//		ui->hintingSelect->setEnabled(true);
//	}
//	else if(radioName == "nativeRadio" )
//	{
//		ui->stackedViews->setCurrentIndex(VIEW_PAGE_ABSOLUTE);
//		ui->hintingSelect->setEnabled(false);
//	}

//	slotView(true);
//}

// void SampleWidget::slotHintChanged(int )
//{
//	slotView(true);
//	emit stateChanged();
// }

// void SampleWidget::slotChangeViewPageSetting ( bool ch )
//{
//	// 	qDebug() <<"MainViewWidget::slotChangeViewPageSetting("<<ch<<")";
//	QString butName ( sender()->objectName() );
//	if ( !ch )
//	{
//		toolPanelWidth = ui->splitter_2->sizes().at ( 1 ) ;
//		ui->stackedTools->hide();
//	}
//	else
//	{
//		ui->stackedTools->show();
//		if ( ui->splitter_2->sizes().at ( 1 ) == 0 )
//		{
//			QList<int> li;
//			li << ui->splitter_2->width() - toolPanelWidth << toolPanelWidth;
//			ui->splitter_2->setSizes ( li );
//		}
//	}

//	QMap<QString, QToolButton*> bmap;
//	QMap<QString, int> pmap;
//	bmap[ "settingsButton" ] = ui->settingsButton;
//	bmap[ "openTypeButton" ] = ui->openTypeButton;
//	bmap[ "sampleButton" ] = ui->sampleButton;
//	pmap[ "settingsButton" ] = VIEW_PAGE_SETTINGS;
//	pmap[ "openTypeButton" ] = VIEW_PAGE_OPENTYPE;
//	pmap[ "sampleButton" ] = VIEW_PAGE_SAMPLES;

//	for (const auto& pk : bmap.keys())
//	{
//		if(butName == pk)
//		{
//			ui->stackedTools->setCurrentIndex(pmap[pk]);
//		}
//		else
//		{
//			bmap[pk]->setChecked ( false );
//		}
//	}
//}

void SampleWidget::slotUpdateSView()
{
    if (ui->loremView->isVisible())
        slotView();
}

void SampleWidget::slotZoom(int z)
{
    double delta = 1.0 + (z / 1000.0);
    QTransform trans;
    trans.scale(delta, delta);

    QGraphicsView *concernedView = nullptr;
    if (ui->loremView_FT->isVisible())
        concernedView = ui->loremView_FT;
    else if (ui->loremView->isVisible()) {
        concernedView = ui->loremView;
        if (delta == 1.0) {
            double horiScaleT(typotek::getInstance()->getDpiX() / 72.0);
            double vertScaleT(typotek::getInstance()->getDpiY() / 72.0);
            QTransform adjustAbsoluteViewT(horiScaleT, 0, 0, vertScaleT, 0, 0);
            trans = adjustAbsoluteViewT;
        }
    }
    // Neither view is visible while the Sample tab is in the background; a
    // restored zoom state reaches this slot in that condition.
    if (!concernedView)
        return;
    concernedView->setTransform(trans, (z == 0) ? false : true);
}

void SampleWidget::slotUpdateRView()
{
    if (ui->loremView_FT->isVisible())
        slotView();
}

void SampleWidget::slotSampleChanged()
{
    typotek::getInstance()->namedSample(ui->sampleTextTree->currentItem()->data(0, Qt::UserRole).toString());
    ui->removeSampleButton->setEnabled(ui->sampleTextTree->currentItem()->parent() == uRoot);
    slotView();
    Q_EMIT stateChanged();
}

void SampleWidget::slotLiveFontSize(double fs)
{
    //	double fs( sampleToolBar->getFontSize() );
    reSize(fs, fs * sampleRatio);
    slotView();
    Q_EMIT stateChanged();
}

void SampleWidget::slotFeatureChanged()
{
    // 	OTFSet ret = deFillOTTree();
    slotView();
    Q_EMIT stateChanged();
}

void SampleWidget::slotDefaultOTF()
{
    OTFSet ots(deFillOTTree());
    typotek *typo(typotek::getInstance());
    typo->setDefaultOTFScript(ots.script);
    typo->setDefaultOTFLang(ots.lang);
    typo->setDefaultOTFGPOS(ots.gpos_features);
    typo->setDefaultOTFGSUB(ots.gsub_features);
}

void SampleWidget::slotResetOTF()
{
    typotek *typo(typotek::getInstance());
    typo->setDefaultOTFScript(QString());
    typo->setDefaultOTFLang(QString());
    typo->setDefaultOTFGPOS(QStringList());
    typo->setDefaultOTFGSUB(QStringList());
}

void SampleWidget::slotChangeScript()
{
    if (ui->useShaperCheck->checkState() == Qt::Checked) {
        slotView();
    }
    Q_EMIT stateChanged();
}

void SampleWidget::slotProgressionChanged()
{
    slotView();
}

void SampleWidget::slotWantShape()
{
    slotView();
    Q_EMIT stateChanged();
}

void SampleWidget::refillSampleList()
{
    ui->sampleTextTree->clear();

    QTreeWidgetItem *curIt = nullptr;
    QMap<QString, QList<QString>> sl = typotek::getInstance()->namedSamplesNames();
    QList<QString> ul(sl.take(QString("User")));
    uRoot = new QTreeWidgetItem(ui->sampleTextTree);
    //: Identify root of user defined sample texts
    uRoot->setText(0, i18nc("@item:inlistbox group of the samples the user made", "User"));
    if (ul.count()) {
        bool first(true);
        for (const auto &uk : std::as_const(ul)) {
            if (first) {
                first = false;
                uRoot->setData(0, Qt::UserRole, QString(QString("User::") + uk));
                curIt = uRoot;
            }
            auto it = new QTreeWidgetItem();
            it->setText(0, uk);
            it->setData(0, Qt::UserRole, QString(QString("User::") + uk));
            uRoot->addChild(it);
        }
    }
    for (const auto slKeys = sl.keys(); const auto &k : slKeys) {
        auto kRoot = new QTreeWidgetItem(ui->sampleTextTree);
        kRoot->setText(0, k);
        bool first(true);
        for (const auto range = sl[k]; const auto &n : range) {
            if (first) {
                first = false;
                kRoot->setData(0, Qt::UserRole, QString(k + QString("::") + n));
                if (!curIt)
                    curIt = kRoot;
            }
            auto it = new QTreeWidgetItem();
            it->setText(0, n);
            it->setData(0, Qt::UserRole, QString(k + QString("::") + n));
            kRoot->addChild(it);
        }
    }

    ui->sampleTextTree->setCurrentItem(curIt);
}

unsigned int SampleWidget::hinting()
{
    //	if(ui->lightHinting->isChecked())
    //		return FT_LOAD_TARGET_LIGHT;
    //	else if(ui->normalHinting->isChecked())
    //		return FT_LOAD_TARGET_NORMAL;

    return FT_LOAD_NO_HINTING;
}

void SampleWidget::slotPrint()
{
    FontItem *font(FMFontDb::DB()->Font(fontIdentifier));
    if (!font)
        return;

    if (printer == nullptr)
        printer = new QPrinter(QPrinter::HighResolution);
    if (printDialog == nullptr) {
        printDialog = new QPrintDialog(printer, this);
        connect(printDialog, qOverload<QPrinter *>(&QPrintDialog::accepted), this, &SampleWidget::slotDoPrinting);
    }

    printDialog->setWindowTitle("Fontmatrix - " + i18nc("@title:window", "Print Sample") + " - " + font->fancyName());
    printDialog->open();
}

void SampleWidget::slotDoPrinting()
{
    layoutForPrint = true;
    doRender();
    printer->setFullPage(true);
    QPainter aPainter(printer);
    loremScene->render(&aPainter);
    layoutForPrint = false;
}

void SampleWidget::slotFileChanged(const QString &)
{
#ifdef PLATFORM_APPLE
    if (fileInfo.lastModified().toMSecsSinceEpoch() == fileLastModified)
        return;
    fileLastModified = fileInfo.lastModified().toMSecsSinceEpoch();
#endif
    // start() restarts a timer that is running
    reloadTimer->start();
}

void SampleWidget::slotReload()
{
    //	reloadTimer->stop();
    slotView();
}

void SampleWidget::slotScriptChange()
{
    slotView();
}

void SampleWidget::saveState()
{
    State s(state());
    QByteArray bs(s.toByteArray());
    FMConfig::setValue(QStringLiteral("Sample/state"), bs);
}

void SampleWidget::slotShowSamples(bool b)
{
    if (b) {
        if (sampleToolBar->isChecked(SampleToolBar::OpenTypeButton)) {
            ui->sampleGridLayout->removeWidget(ui->opentypeWidget);
            ui->opentypeWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_OPENTYPE));
            sampleToolBar->toggle(SampleToolBar::OpenTypeButton, false);
        }
        if (sampleToolBar->isChecked(SampleToolBar::VariationsButton))
            sampleToolBar->toggle(SampleToolBar::VariationsButton, false);
        ui->sampleEditWidget->setAutoFillBackground(true);
        ui->sampleEditWidget->resize(ui->sampleGridLayout->geometry().width() / 2, ui->sampleGridLayout->geometry().height());
        ui->sampleGridLayout->addWidget(ui->sampleEditWidget, 0, 0, Qt::AlignRight);
    } else {
        ui->sampleGridLayout->removeWidget(ui->sampleEditWidget);
        ui->sampleEditWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_SAMPLES));
    }
}

void SampleWidget::slotShowOpenType(bool b)
{
    if (b) {
        if (sampleToolBar->isChecked(SampleToolBar::SampleButton)) {
            ui->sampleGridLayout->removeWidget(ui->sampleEditWidget);
            ui->sampleEditWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_SAMPLES));
            sampleToolBar->toggle(SampleToolBar::SampleButton, false);
        }
        if (sampleToolBar->isChecked(SampleToolBar::VariationsButton))
            sampleToolBar->toggle(SampleToolBar::VariationsButton, false);
        ui->opentypeWidget->setAutoFillBackground(true);
        ui->opentypeWidget->resize(ui->sampleGridLayout->geometry().width() / 2, ui->sampleGridLayout->geometry().height());
        ui->sampleGridLayout->addWidget(ui->opentypeWidget, 0, 0, Qt::AlignRight);
    } else {
        ui->sampleGridLayout->removeWidget(ui->opentypeWidget);
        ui->opentypeWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_OPENTYPE));
    }
}

void SampleWidget::slotShowVariations(bool b)
{
    if (b) {
        // the other two panels go back to their pages
        if (sampleToolBar->isChecked(SampleToolBar::SampleButton))
            sampleToolBar->toggle(SampleToolBar::SampleButton, false);
        if (sampleToolBar->isChecked(SampleToolBar::OpenTypeButton))
            sampleToolBar->toggle(SampleToolBar::OpenTypeButton, false);
        variationsWidget->setAutoFillBackground(true);
        variationsWidget->resize(ui->sampleGridLayout->geometry().width() / 2, ui->sampleGridLayout->geometry().height());
        ui->sampleGridLayout->addWidget(variationsWidget, 0, 0, Qt::AlignRight);
    } else {
        ui->sampleGridLayout->removeWidget(variationsWidget);
        variationsWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_VARIATIONS));
    }
}

void SampleWidget::slotAddSample()
{
    QString nu(i18nc("@item:inlistbox default name of a new sample text", "New Sample"));
    newSampleName = new QTreeWidgetItem();
    newSampleName->setText(0, nu);
    newSampleName->setData(0, Qt::UserRole, QString("NEW_SAMPLE"));
    newSampleName->setFlags(newSampleName->flags() | Qt::ItemIsEditable);
    uRoot->addChild(newSampleName);
    ui->sampleTextTree->openPersistentEditor(newSampleName);
    if (!uRoot->isExpanded())
        uRoot->setExpanded(true);
    ui->sampleTextTree->setCurrentItem(newSampleName);
}

void SampleWidget::slotSampleNameEdited(QWidget *)
{
    ui->sampleTextTree->closePersistentEditor(newSampleName);
    newSampleName->setData(0, Qt::UserRole, QString(QString("User::") + newSampleName->text(0)));
    typotek::getInstance()->changeSample(newSampleName->text(0), ui->sampleEdit->toPlainText());
}

void SampleWidget::slotRemoveSample()
{
    QString name(ui->sampleTextTree->currentItem()->text(0));
    QTreeWidgetItem *currentItem = ui->sampleTextTree->currentItem();
    uRoot->removeChild(currentItem);
    typotek::getInstance()->removeNamedSample(name);
}

void SampleWidget::slotEditSample()
{
    disconnect(ui->sampleEdit, &QPlainTextEdit::textChanged, this, &SampleWidget::slotUpdateSample);
    QTreeWidgetItem *currentItem = ui->sampleTextTree->currentItem();
    ui->sampleEdit->setPlainText(typotek::getInstance()->namedSample(currentItem->data(0, Qt::UserRole).toString()));
    ui->sampleEdit->setReadOnly(currentItem->parent() != uRoot);
    connect(ui->sampleEdit, &QPlainTextEdit::textChanged, this, &SampleWidget::slotUpdateSample);
}

void SampleWidget::slotUpdateSample()
{
    typotek::getInstance()->changeSample(ui->sampleTextTree->currentItem()->text(0), ui->sampleEdit->toPlainText());
    slotView();
}

#include "moc_samplewidget.cpp"
