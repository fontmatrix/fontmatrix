/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmpreviewlist.h"
#include "fontmatrix_debug.h"

#include "fmfloatingpreview.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "mainviewwidget.h"
#include "typotek.h"

#include "fmconfig.h"
#include <KLocalizedString>
#include <QDebug>
#include <QImage>
#include <QScrollBar>
// #include <QDrag>
// #include <QMimeData>
#include <QApplication>
#include <QBrush>
#include <QLabel>
#include <QPainter>

constexpr int FM_MINIMUM_PREVIEW_WIDTH = 280;

bool FMPreviewIconEngine::initState = false;
QPen FMPreviewIconEngine::pen = QPen();
QVector<QRgb> FMPreviewIconEngine::m_selPalette;
QRgb FMPreviewIconEngine::activatedColor = qRgb(9, 223, 11);
QRgb FMPreviewIconEngine::deactivatedColor = qRgb(190, 190, 190);
QRgb FMPreviewIconEngine::partlyActivatedColor = qRgb(166, 220, 220);

FMPreviewIconEngine::FMPreviewIconEngine()
    : activatedFont(NotActivated)
{
    if (!initState) {
        // setup palette
        QColor sColor(QApplication::palette().color(QPalette::Highlight));
        QColor tColor(QApplication::palette().color(QPalette::HighlightedText));
        m_selPalette.clear();
        int sr(sColor.red());
        int sg(sColor.green());
        int sb(sColor.blue());
        int tr(tColor.red());
        int tg(tColor.green());
        int tb(tColor.blue());
        int cpal(256);
        for (int aa = 0; aa < cpal; ++aa) {
            int sn(cpal - aa);
            int tn(aa);
            m_selPalette << qRgb(((sr * sn) + (tr * tn)) / cpal, ((sg * sn) + (tg * tn)) / cpal, ((sb * sn) + (tb * tn)) / cpal);
        }

        // setup "writing" pen
        //		pen.setColor(QColor(m_selPalette.at(128)));
        pen.setColor(QColor(220, 220, 220));
        pen.setWidth(1);

        initState = true;
    }
}

QIconEngine *FMPreviewIconEngine::clone() const
{
    // QIcon calls this when a shared icon is modified; a null engine crashes it
    return new FMPreviewIconEngine(*this);
}

QVector<QRgb> FMPreviewIconEngine::actualSelPalette(const QVector<QRgb> &orig)
{
    QRgb r(QApplication::palette().color(QPalette::Highlight).rgb());
    QVector<QRgb> ret;
    for (int i(0); i < 256; ++i)
        ret << r;
    QColor bgColor(QApplication::palette().color(QPalette::Base));
    QColor fgColor(QApplication::palette().color(QPalette::Text));

    // In m_selPalette, background is at the begining of the vector

    bool DarkOnLight(fgColor.rgb() < bgColor.rgb());
    // order is dark first, light last
    QMap<QRgb, int> order;
    for (int i(0); i < orig.size(); ++i)
        order[orig[i]] = i;
    QList<int> oIdx(order.values());
    if (DarkOnLight) {
        // oIdx has background values at the end
        int v(0);
        for (int c(oIdx.size() - 1); c >= 0; --c) {
            ret[oIdx[c]] = m_selPalette.at(v);
            ++v;
        }
    } else {
        int v(0);
        for (int c(0); c < oIdx.size(); c++) {
            ret[oIdx[c]] = m_selPalette.at(v);
            ++v;
        }
    }
    return ret;
}

FMPreviewIconEngine::~FMPreviewIconEngine()
{
    //	if(m_p)
    //		delete m_p;
}

void FMPreviewIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State)
{
    if (!m_p.isNull()) {
        painter->save();
        painter->translate(rect.x(), rect.y());
        QRect r(0, 0, rect.width(), rect.height());
        QPainterPath pp;
        double rr(double(r.height()) / 5.0);
        pp.addRoundedRect(r, rr, rr);
        painter->setRenderHint(QPainter::Antialiasing, true);
        // draw background
        painter->save();
        painter->setPen(Qt::NoPen);
        if (mode == QIcon::Selected)
            painter->setBrush(QApplication::palette().color(QPalette::Highlight));
        else
            painter->setBrush(QApplication::palette().color(QPalette::Base));
        painter->drawPath(pp);
        painter->restore();
        // end of bg
        painter->setPen(pen);
        QRect tr(r);
        tr.translate(0, (r.height() - m_p.height()) / 2);
        if (mode == QIcon::Selected) {
            QImage hm(m_p.toImage().convertToFormat(QImage::Format_Indexed8));
            hm.setColorTable(actualSelPalette(hm.colorTable()));
            painter->setClipPath(pp);
            painter->drawPixmap(tr, QPixmap::fromImage(hm), r);
            painter->drawPath(pp);
        } else {
            painter->drawPixmap(tr, m_p, r);
            painter->drawPath(pp);
        }
        if (activatedFont != NotActivated) {
            painter->setPen(Qt::NoPen);
            QPainterPath activationPath;
            double rr2(rr / 2.0);
            activationPath.moveTo(rr, 0);
            activationPath.cubicTo(rr2, 0, 0, rr2, 0, rr);
            activationPath.lineTo(0, rect.height() - rr);
            activationPath.cubicTo(0, rect.height() - rr2, rr2, rect.height(), rr, rect.height());
            activationPath.closeSubpath();
            if (activatedFont == Activated)
                painter->setBrush(QBrush(activatedColor));
            else if (activatedFont == PartlyActivated)
                painter->setBrush(QBrush(partlyActivatedColor));
            painter->drawPath(activationPath);
        }
        painter->restore();
    }
}

void FMPreviewIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode, QIcon::State)
{
    m_p = pixmap;
}

FMPreviewModel::FMPreviewModel(QObject *pa, FMPreviewView *wPa, QList<FontItem *> db)
    : QAbstractListModel(pa)
    , m_view(wPa)
    , base(db)
{
    familyMode = false;
    styleTooltipName = FMConfig::value(QStringLiteral("Preview/StyleTooltipName"), QStringLiteral("font-weight:bold;")).toString();
    styleTooltipPath = FMConfig::value(QStringLiteral("Preview/StyleTooltipPath"), QStringLiteral("font-weight:normal;font-size:small;")).toString();
    styleTooltipTags =
        FMConfig::value(QStringLiteral("Preview/StyleTooltipTags"), QStringLiteral("text-align:right;font-weight:normal;font-size:small;font-style:italic;"))
            .toString();

    FMConfig::setValue(QStringLiteral("Preview/StyleTooltipName"), styleTooltipName);
    FMConfig::setValue(QStringLiteral("Preview/StyleTooltipPath"), styleTooltipPath);
    FMConfig::setValue(QStringLiteral("Preview/StyleTooltipTags"), styleTooltipTags);
}

QVariant FMPreviewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    // 	qDebug()<<"D"<<row;
    FontItem *fit;
    FontNamedInstance instance;
    int instanceIndex(-1);
    if (instanceMode && !rows.isEmpty()) {
        if (row >= rows.size())
            return QVariant();
        fit = rows.at(row).font;
        instanceIndex = rows.at(row).instance;
        if (fit && instanceIndex >= 0)
            instance = fit->namedInstances().value(instanceIndex);
    } else if (base.isEmpty())
        fit = FMFontDb::DB()->getFilteredFonts(true).at(row);
    else
        fit = base.at(row);
    if (!fit)
        return QVariant();

    QColor bgColor(QApplication::palette().color(QPalette::Base));
    QColor fgColor(QApplication::palette().color(QPalette::Text));

    int width(m_view->getUsedWidth());

    if (role == Qt::DisplayRole) {
        if (typotek::getInstance()->getPreviewSubtitled())
            return instanceIndex >= 0 ? QString(fit->family() + QLatin1Char(' ') + instance.name) : fit->fancyName();
        else
            return QVariant();
    } else if (role == Qt::DecorationRole) {
        QString word;
        if (instanceIndex >= 0) {
            // the style is the one of the instance, not the one of the file
            word = typotek::getInstance()->word(nullptr);
            if (word.isEmpty())
                word = specString;
            word.replace(QLatin1String("<variant>"), instance.name);
            word.replace(QLatin1String("<name>"), QString(fit->family() + QLatin1Char(' ') + instance.name));
            word.replace(QLatin1String("<family>"), fit->family());
        } else if (specString.isEmpty())
            word = typotek::getInstance()->word(fit);
        else
            word = typotek::getInstance()->word(fit, specString);
        QPixmap im(fit->oneLinePreviewPixmap(word, fgColor, bgColor, width, 0, instance.coords));
        auto pie(new FMPreviewIconEngine);
        if (!familyMode)
            pie->setActivation(fit->isActivated() ? FMPreviewIconEngine::Activated : FMPreviewIconEngine::NotActivated);
        else {
            bool hasActive(false);
            bool hasNotActive(false);
            for (const auto familySet = FMFontDb::DB()->FamilySet(fit->family()); auto *f : familySet) {
                if (f->isActivated())
                    hasActive = true;
                else
                    hasNotActive = true;
                if (hasActive && hasNotActive)
                    break;
            }
            if (hasNotActive && hasActive)
                pie->setActivation(FMPreviewIconEngine::PartlyActivated);
            else {
                pie->setActivation(hasActive ? FMPreviewIconEngine::Activated : FMPreviewIconEngine::NotActivated);
            }
        }
        QIcon ic(pie);
        ic.addPixmap(im);

        return ic;
    } else if (role == Qt::ToolTipRole) {
        if (familyMode) {
            QList<FontItem *> fam(FMFontDb::DB()->FamilySet(fit->family()));
            QString sRet;
            sRet += "<div style=\"" + styleTooltipName + "\">" + fit->family() + " (" + QString::number(fam.size()) + ")</div>";
            sRet += "<div style=\"" + styleTooltipTags + "\">" + fit->tags().join(QString(", ")) + "</div>";

            for (auto *ffi : std::as_const(fam)) {
                QString style(ffi->variant());
                if (ffi->isVariable()) {
                    // the named instances of a variable font are its styles
                    QStringList names;
                    const QList<FontNamedInstance> instances(ffi->namedInstances());
                    for (const FontNamedInstance &instance : instances)
                        names << instance.name;
                    style = i18nc("@info:tooltip a variable font and its named instances, %1 the style name, %2 the instances",
                                  "%1 (variable: %2)",
                                  ffi->variant(),
                                  names.join(QStringLiteral(", ")));
                }
                sRet += "<div style=\"" + styleTooltipPath + "\">" + style + "</div>";
            }
            return sRet;
        }
        if (typotek::getInstance()->getPreviewSubtitled()) {
            return QString("<div style=\"" + styleTooltipPath + "\">" + fit->path() + "</div>");
        } else {
            QString complete;
            complete += "<div style=\"" + styleTooltipName + "\">" + fit->fancyName() + "</div>";
            complete += "\n";
            complete += "<div style=\"" + styleTooltipPath + "\">" + fit->path() + "</div>";
            return complete;
        }
    } else if (role == PathRole) {
        return fit->path();
    } else if (role == InstanceRole) {
        return instanceIndex;
    }

    // fall back
    return QVariant();
}

Qt::ItemFlags FMPreviewModel::flags(const QModelIndex &) const
{
    return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

int FMPreviewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !typotek::getInstance()->getTheMainView())
        return 0;
    int cl(0);
    if (instanceMode && !rows.isEmpty())
        cl = rows.size();
    else if (base.isEmpty())
        cl = FMFontDb::DB()->getFilteredFonts(true).size();
    else
        cl = base.size();
    return cl;
}

void FMPreviewModel::dataChanged()
{
    // qualified: this class has a dataChanged() of its own, which hides the signal
    Q_EMIT QAbstractItemModel::dataChanged(index(0), index(rowCount(QModelIndex()) - 1));
    m_view->updateLayout();
    Q_EMIT layoutChanged();
}

void FMPreviewModel::resetBase(QList<FontItem *> db)
{
    base = db;
    rows.clear();
    if (instanceMode) {
        for (FontItem *font : std::as_const(base)) {
            const int instances(font && font->isVariable() ? int(font->namedInstances().size()) : 0);
            if (instances == 0)
                rows << Row{font, -1};
            for (int i(0); i < instances; ++i)
                rows << Row{font, i};
        }
    }
    dataChanged();
}

QModelIndex FMPreviewModel::indexOf(FontItem *font, int instance) const
{
    if (!instanceMode || rows.isEmpty()) {
        const int row(base.indexOf(font));
        return row < 0 ? QModelIndex() : index(row);
    }
    for (int row(0); row < rows.size(); ++row) {
        if (rows.at(row).font == font && rows.at(row).instance == instance)
            return index(row);
    }
    return QModelIndex();
}

QList<FontItem *> FMPreviewModel::getBase()
{
    if (base.isEmpty())
        return FMFontDb::DB()->getFilteredFonts(true);
    else
        return base;
}

FMPreviewView::FMPreviewView(QWidget *parent)
    : QListView(parent)
    , columns(1)
{
    dragFlag = false;
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionRectVisible(false);
}

bool FMPreviewView::moveTo(const QString &fname)
{
    QList<FontItem *> fl(reinterpret_cast<FMPreviewModel *>(model())->getBase());

    QString uname(fname.toUpper());
    const int fl_count(fl.size());
    int rFont(fl_count);
    for (int i(0); i < fl_count; ++i) {
        QString pname(fl[i]->fancyName().toUpper());
        pname.truncate(uname.size());
        if (uname == pname) {
            rFont = i;
            break;
        }
    }

    if (rFont != fl_count) {
        auto mod(reinterpret_cast<QAbstractListModel *>(model()));
        QModelIndex mi(mod->index(rFont));
        if (mi.isValid()) {
            selectionModel()->setCurrentIndex(mi, QItemSelectionModel::ClearAndSelect);
            // scrollTo ( mi );
            return true;
        }
    }

    return false;
}

void FMPreviewView::resizeEvent(QResizeEvent *event)
{
    int actualWidth(width() - 20); // if we use the viewport size, it becomes funny when a resize shows/hides the scrollbar
    setSpacing(0);
    int gHeight(2.0 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0);
    qCDebug(FONTMATRIX_LOG) << "VW" << actualWidth << verticalScrollBar()->width() << "S" << spacing();
    double cNr(1);

    if (columns == 1)
        usedWidth = qRound((double(actualWidth) / columns));
    else {
        int minCellWidth(FM_MINIMUM_PREVIEW_WIDTH + 6);
        cNr = qRound(double(actualWidth) / minCellWidth);
        minCellWidth = qRound((double(actualWidth) / cNr) - 6);
        qCDebug(FONTMATRIX_LOG) << "C" << cNr << "U" << minCellWidth;
        setGridSize(QSize(minCellWidth, gHeight + 12));
        usedWidth = minCellWidth - 6;
    }
    setIconSize(QSize(usedWidth, gHeight + 6));
    QListView::resizeEvent(event);
}

void FMPreviewView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        startDragPoint = event->pos();
        dragFlag = false;
        //		const QModelIndex idx ( indexAt(startDragPoint) );
        //		if(idx.isValid())
        //			emit pressed(idx);
    }
    QListView::mousePressEvent(event);
}

void FMPreviewView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->modifiers().testFlag(Qt::ControlModifier)))
        return;
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - startDragPoint).manhattanLength() < QApplication::startDragDistance())
        return;

    auto m(reinterpret_cast<FMPreviewModel *>(model()));
    if (m && m->getFamilyMode())
        return;
    // Create a window with the current preview
    if (currentIndex().isValid() && (!dragFlag)) {
        dragFlag = true;
        //		FontItem * sf(typotek::getInstance()->getSelectedFont());
        QModelIndex idx = indexAt(startDragPoint);
        QString fname = idx.data(FMPreviewModel::PathRole).toString();
        if (!fname.isEmpty()) {
            FontItem *sf(FMFontDb::DB()->Font(fname));
            if (sf)
                FMFloatingPreview::create(sf, QRect(event->globalPosition().toPoint(), QSize(width(), 1)));
        }
    }
}

void FMPreviewView::keyPressEvent(QKeyEvent *event)
{
    qCDebug(FONTMATRIX_LOG) << "FMPreviewView::keyPressEvent" << event;
    if ((!event->text().isEmpty()) && (event->text().at(0).isLetterOrNumber()))
        Q_EMIT keyPressed(event->text());
    else
        QListView::keyPressEvent(event);
}

void FMPreviewView::updateLayout()
{
    update();
}

void FMPreviewView::setCurrentFont(const QString &name)
{
    QList<FontItem *> fl(reinterpret_cast<FMPreviewModel *>(model())->getBase());

    const int fl_count(fl.size());
    int rFont(fl_count);
    for (int i(0); i < fl_count; ++i) {
        if (fl[i]->path() == name) {
            rFont = i;
            break;
        }
    }

    if (rFont != fl_count) {
        auto mod(reinterpret_cast<QAbstractListModel *>(model()));
        QModelIndex mi(mod->index(rFont));
        if (mi.isValid()) {
            selectionModel()->setCurrentIndex(mi, QItemSelectionModel::ClearAndSelect);
            // scrollTo ( mi );
        }
    }
}

#include "moc_fmpreviewlist.cpp"
