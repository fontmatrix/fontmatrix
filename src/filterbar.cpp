/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterbar.h"
#include "filterduplicate.h"
#include "filteritem.h"
#include "filterkind.h"
#include "filterlang.h"
#include "filterlicense.h"
#include "filtermeta.h"
#include "filterpanose.h"
#include "filtersdialog.h"
#include "filtersdialogitem.h"
#include "filtertag.h"
#include "fmduplicates.h"
#include "fmfontdb.h"
#include "fmfontkinds.h"
#include "fmlangcoverage.h"
#include "fmlicense.h"
#include "fmpaths.h"
#include "fontmatrix_debug.h"
#include "mainviewwidget.h"
#include "metawidget.h"
#include "panosewidget.h"
#include "typotek.h"
#include "ui_filterbar.h"

#include "fmconfig.h"
#include <KLocalizedString>
#include <QAction>
#include <QApplication>
#include <QCompleter>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QInputDialog>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QStringList>

const QString &FilterBar::andOp()
{
    static const QString s = i18nc("@item:intext filter operator", "And");
    return s;
}
const QString &FilterBar::notOp()
{
    static const QString s = i18nc("@item:intext filter operator", "Not");
    return s;
}
const QString &FilterBar::orOp()
{
    static const QString s = i18nc("@item:intext filter operator", "Or");
    return s;
}

TagListModel::TagListModel(QObject *parent)
    : QAbstractListModel(parent)
    , specialTagsCount(1)
{
    //	ui->tagsCombo->clear();
    //	//	tagsetIcon = QIcon(":/fontmatrix_tagseteditor.png");

    //	ui->tagsCombo->addItem(i18n("Tags"),"NO_KEY");
    //	ui->tagsCombo->addItem(i18n("All activated"),"ALL_ACTIVATED");

    //	QStringList tl_tmp = FMFontDb::DB()->getTags();
    ////	qDebug()<< "T"<< tl_tmp.join("||");
    //	tl_tmp.sort();
    //	for (const auto& tag : tl_tmp)
    //	{
    //		if(!FMFontDb::DB()->Fonts(tag, FMFontDb::Tags ).isEmpty())
    //			ui->tagsCombo->addItem(tag, "TAG");
    //	}
}

int TagListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return FMFontDb::DB()->getTags().count() + specialTagsCount;
}

int TagListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 1; // let's start simple
}

QVariant TagListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return QVariant();
    // getTags() is cached and sorted, the specials come first
    const bool special(index.row() < specialTagsCount);
    const QString tag(special ? i18nc("@item:inlistbox pseudo tag that lists the activated fonts", "Activated")
                              : FMFontDb::DB()->getTags().value(index.row() - specialTagsCount));
    if (role == Qt::DisplayRole) {
        //		return tag;
        return QVariant();
        //		return QString("<div style=\"color:red;\">%1</div>").arg(tag);
    } else if (role == Qt::EditRole)
        return tag;
    else if (role == Qt::DecorationRole) {
        //		return QVariant();
        QString ts("%1 (%2)");
        int tc(special ? FMFontDb::DB()->Fonts(1, FMFontDb::Activation).count() : FMFontDb::DB()->Fonts(tag, FMFontDb::Tags).count());
        QRect pr(0, 0, 1024, 18);
        QPixmap pm(pr.size());
        QPainter p;
        p.begin(&pm);
        const QString label(ts.arg(tag, QString::number(tc)));
        p.drawText(pr, Qt::AlignLeft | Qt::TextDontClip | Qt::TextSingleLine, label, &pr);
        p.end();
        QPixmap tagPix(pr.width() + 18, 18);
        tagPix.fill(Qt::transparent);
        p.begin(&tagPix);
        p.setRenderHint(QPainter::Antialiasing);
        p.save();
        p.setBrush(QColor(210, 210, 210));
        if (currentTags.contains(tag))
            p.setBrush(QColor(180, 180, 180));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(tagPix.rect(), 5, 5);
        p.restore();
        pr.translate(9, 0);
        p.drawText(pr, label);
        p.end();
        return tagPix;
    } else if (role == TagType) {
        if (index.row() < specialTagsCount) {
            return QString("ALL_ACTIVATED");
        }
        return QString("TAG");
    } else if (role == TagString) {
        return tag;
    }
    return QVariant();
}

bool TagListModel::setData(const QModelIndex &index, const QVariant &value, int)
{
    if (!index.isValid())
        return false;
    if (value.toString().isEmpty())
        return false;
    if (index.row() < specialTagsCount)
        return false;
    const QStringList tl_tmp = FMFontDb::DB()->getTags();
    if (value.toString() == tl_tmp.at(index.row() - specialTagsCount))
        return false;
    FMFontDb::DB()->editTag(tl_tmp.at(index.row() - specialTagsCount), value.toString());
    Q_EMIT dataChanged(index, index);
    return true;
}

Qt::ItemFlags TagListModel::flags(const QModelIndex &) const
{
    //	if(index.row() > specialTagsCount - 1)
    //		return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void TagListModel::clearCurrents()
{
    if (!currentTags.isEmpty()) {
        currentTags.clear();
        Q_EMIT dataChanged(index(0, 0), index(FMFontDb::DB()->getTags().count() + specialTagsCount - 1, columnCount() - 1));
    }
}

void TagListModel::addToCurrents(const QString &t)
{
    if (!currentTags.contains(t)) {
        currentTags << t;
        Q_EMIT dataChanged(index(0, 0), index(FMFontDb::DB()->getTags().count() + specialTagsCount - 1, columnCount() - 1));
    }
}

void TagListModel::removeFromCurrents(const QString &t)
{
    if (currentTags.contains(t)) {
        currentTags.removeAll(t);
        Q_EMIT dataChanged(index(0, 0), index(FMFontDb::DB()->getTags().count() + specialTagsCount - 1, columnCount() - 1));
    }
}

void TagListModel::renameCurrent(const QString &from, const QString &to)
{
    const int idx(currentTags.indexOf(from));
    if (idx >= 0)
        currentTags[idx] = to;
}

void TagListModel::tagsDBChanged()
{
    // The number of rows may have changed, dataChanged() is not enough
    beginResetModel();
    endResetModel();
}

FilterBar::FilterBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FilterBar)
{
    ui->setupUi(this);

    tagListModel = new TagListModel(this);
    //	ui->tagsView->setShowGrid(false);
    //	ui->tagsView->horizontalHeader()->setStretchLastSection(true);
    //	ui->tagsView->verticalHeader()->hide();
    //	ui->tagsView->horizontalHeader()->hide();
    ui->tagsView->setModel(tagListModel);

    //	metaFieldsMenu = new QMenu(i18n("Fields"), this);
    QList<FMFontDb::InfoItem> ln;
    metaFieldKey = int(FMFontDb::AllInfo);
    ln << FMFontDb::AllInfo << FMFontDb::FontFamily << FMFontDb::FontSubfamily << FMFontDb::Designer << FMFontDb::Description << FMFontDb::Copyright
       << FMFontDb::Trademark << FMFontDb::ManufacturerName << FMFontDb::LicenseDescription;
    for (int gIdx(0); gIdx < ln.count(); ++gIdx) {
        FMFontDb::InfoItem k(ln[gIdx]);
        {
            QString fieldname(FontStrings::Names().value(k));
            //			QAction * ma(new QAction(fieldname, metaFieldsMenu));
            //			ma->setData(int(k));
            //			metaFieldsMenu->addAction(ma);
            ui->fieldCombo->addItem(fieldname, int(k));
        }
    }
    //	ui->metadataTool->setMenu(metaFieldsMenu);
    //	ui->metaFieldLabel->setText(FontStrings::Names().value(FMFontDb::AllInfo));
    mModel = new QStringListModel;
    mModel->setStringList(mList);
    ui->metadataLineEdit->setCompleter(new QCompleter(mModel));

    loadFilters();

    connect(ui->tagsView, &TagListView::clicked, this, &FilterBar::slotTagSelect);
    connect(ui->tagsView, &TagListView::doubleClicked, this, &FilterBar::slotTagEdit);

    connect(ui->metadataLineEdit, &QLineEdit::editingFinished, this, &FilterBar::metaFilter);
    connect(ui->fieldCombo, &QComboBox::activated, this, &FilterBar::metaSelectField);
    //	connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(slotClearFilter()));
    connect(ui->panoseWidget, &PanoseWidget::filterChanged, this, &FilterBar::slotPanoFilter);
    connect(FMFontDb::DB(), &FMFontDb::tagsChanged, tagListModel, &TagListModel::tagsDBChanged);

    // the languages a font can set are fontconfig's answer: without it the section has nothing to say
    ui->languagesBox->setVisible(FMLangCoverage::isAvailable());
    ui->languagesArrow->setVisible(FMLangCoverage::isAvailable());
    ui->line_5->setVisible(FMLangCoverage::isAvailable());

    connect(ui->languagesCombo, &QComboBox::activated, this, &FilterBar::slotLangFilter);
    connect(ui->licenseCombo, &QComboBox::activated, this, &FilterBar::slotLicenseFilter);
    connect(ui->duplicatesCombo, &QComboBox::activated, this, &FilterBar::slotDuplicateFilter);
    connect(ui->duplicatesArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotToggleDuplicates);
    connect(ui->duplicatesGroupsButton, &QPushButton::clicked, typotek::getInstance(), &typotek::slotShowDuplicates);
    connect(ui->kindCombo, &QComboBox::activated, this, &FilterBar::slotKindFilter);
    connect(ui->kindArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotToggleKinds);
    connect(ui->languagesArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotToggleLanguages);
    connect(ui->licenseArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotToggleLicense);

    connect(ui->tagsArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotToggleTags);
    connect(ui->metadataArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotToggleMeta);
    connect(ui->panoseArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotTogglePano);
    connect(ui->filtersArrow, &OpenCloseArrow::openChanged, this, &FilterBar::slotToggleFilter);

    ui->tagsArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/TagsOpen"), true).toBool());
    ui->metadataArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/MetaOpen"), false).toBool());
    ui->panoseArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/PanoseOpen"), false).toBool());
    ui->filtersArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/FiltersOpen"), true).toBool());
    ui->languagesArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/LanguagesOpen"), false).toBool());
    ui->licenseArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/LicenseOpen"), false).toBool());
    ui->duplicatesArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/DuplicatesOpen"), false).toBool());
    ui->kindArrow->changeOpen(FMConfig::value(QStringLiteral("FilterBar/KindOpen"), false).toBool());
}

FilterBar::~FilterBar()
{
    FMConfig::setValue(QStringLiteral("FilterBar/TagsOpen"), ui->tagsArrow->isOpen());
    FMConfig::setValue(QStringLiteral("FilterBar/MetaOpen"), ui->metadataArrow->isOpen());
    FMConfig::setValue(QStringLiteral("FilterBar/PanoseOpen"), ui->panoseArrow->isOpen());
    FMConfig::setValue(QStringLiteral("FilterBar/FiltersOpen"), ui->filtersArrow->isOpen());
    FMConfig::setValue(QStringLiteral("FilterBar/LanguagesOpen"), ui->languagesArrow->isOpen());
    FMConfig::setValue(QStringLiteral("FilterBar/LicenseOpen"), ui->licenseArrow->isOpen());
    FMConfig::setValue(QStringLiteral("FilterBar/DuplicatesOpen"), ui->duplicatesArrow->isOpen());
    FMConfig::setValue(QStringLiteral("FilterBar/KindOpen"), ui->kindArrow->isOpen());
    delete ui;
}

void FilterBar::changeEvent(QEvent *e)
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

void FilterBar::metaFilter()
{
    if (ui->metadataLineEdit->text().isEmpty())
        return;

    auto fm(new FilterMeta);

    if (!mList.contains(ui->metadataLineEdit->text())) {
        mList.append(ui->metadataLineEdit->text());
        mModel->setStringList(mList);
    }

    fm->setData(FilterData::Text,
                QString(FontStrings::Names().value(static_cast<FMFontDb::InfoItem>(metaFieldKey)) + QString(" : ") + ui->metadataLineEdit->text()));
    fm->setData(FilterMeta::Field, metaFieldKey);
    fm->setData(FilterMeta::Value, ui->metadataLineEdit->text());
    //				fm->setData(FilterData::Or, false);
    //				fm->setData(FilterData::And, true);
    addFilterItem(fm, false);

    ui->metadataLineEdit->clear();
    processFilters();
}

void FilterBar::invalidateCoverage()
{
    // fonts were added or removed: the languages and the licences are read again
    FMLangCoverage::invalidate();
    FMLicense::invalidate();
    FMDuplicates::invalidate();
    FMFontKinds::invalidate();
    languagesFilled = false;
    licensesFilled = false;
    duplicatesFilled = false;
    kindsFilled = false;
    if (ui->languagesArrow->isOpen())
        fillLanguages();
    if (ui->licenseArrow->isOpen())
        fillLicenses();
    if (ui->duplicatesArrow->isOpen())
        fillDuplicates();
    if (ui->kindArrow->isOpen())
        fillKinds();
}

void FilterBar::refilter()
{
    if (filters.count() > 0)
        processFilters();
    else {
        FMFontDb::DB()->filterAllFonts();
        Q_EMIT filterChanged();
    }
}

void FilterBar::processFilters()
{
    if (filters.count() > 0) {
        FMFontDb::DB()->clearFilteredFonts();
        bool first(true);
        for (auto *d : std::as_const(filters)) {
            if (first) {
                d->hideOperation(FilterItem::AND);
                first = false;
            }
            d->filter()->operate();
        }
        Q_EMIT filterChanged();
    }
}

void FilterBar::slotRemoveFilterItem(bool process)
{
    auto fi(reinterpret_cast<FilterItem *>(sender()));
    if (fi != nullptr) {
        filters.removeAll(fi);
        if (filters.count() == 0) {
            FMFontDb::DB()->filterAllFonts();
            curFilterWidget->setVisible(false);
            Q_EMIT filterChanged();
        }
        fi->deleteLater();
        if (process && (filters.count() > 0))
            processFilters();
    }
}

void FilterBar::removeAllFilters()
{
    FMFontDb::DB()->filterAllFonts();
    for (auto *d : std::as_const(filters)) {
        d->deleteLater();
    }
    filters.clear();
    curFilterWidget->setVisible(false);
}

void FilterBar::addFilterItem(FilterData *f, bool process)
{
    if (f != nullptr) {
        curFilterWidget->setVisible(true);
        FilterItem *it(f->item());
        if (!filters.contains(it)) {
            if (f->data(FilterData::Replace).toBool())
                removeAllFilters();
            connect(it, &FilterItem::remove, this, [this]() {
                slotRemoveFilterItem();
            });
            connect(f, &FilterData::Changed, this, &FilterBar::processFilters);
            filters.append(it);
            filterListLayout->addWidget(it);

            if (process)
                processFilters();
        }
    }
}

QString FilterBar::filterString(FilterData *d, bool first)
{
    QString fs;
    if (first) {
        if (d->data(FilterData::Not).toBool())
            fs += notOp() + QString(" [%1] ").arg(d->getText());
        else
            fs += QString("[%1] ").arg(d->getText());
    } else {
        if (d->data(FilterData::Or).toBool())
            fs += orOp();
        else
            fs += andOp();

        if (d->data(FilterData::Not).toBool())
            fs += QString(" %1").arg(notOp());
        fs += QString(" [%1] ").arg(d->getText());
    }
    return fs;
}

void FilterBar::loadFilters()
{
    for (auto *i : std::as_const(items))
        delete i;
    items.clear();

    QDir fbasedir(FMPaths::FiltersDir());
    QStringList fbaselist(fbasedir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name));
    for (const auto &fname : std::as_const(fbaselist)) {
        QDir fdir(FMPaths::FiltersDir() + fname);
        QStringList flist(fdir.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::Name));
        QString fString;
        bool first(true);
        for (const auto &fn : std::as_const(flist)) {
            QStringList l(fn.split(QString("-")));
            if (l.count() == 2) {
                QString type(l.at(1));
                QFile file(fdir.absoluteFilePath(fn));
                if (file.open(QIODevice::ReadOnly)) {
                    FilterData *f = nullptr;
                    if (type == QString("Meta")) {
                        f = new FilterMeta;
                    } else if (type == QString("Panose")) {
                        f = new FilterPanose;
                    } else if (type == QString("Tag")) {
                        f = new FilterTag;
                    } else if (type == QString("Lang")) {
                        f = new FilterLang;
                    } else if (type == QString("License")) {
                        f = new FilterLicense;
                    } else if (type == QString("Duplicate")) {
                        f = new FilterDuplicate;
                    } else if (type == QString("Kind")) {
                        f = new FilterKind;
                    }
                    if (!f)
                        continue;
                    f->fromByteArray(file.readAll());
                    if (first) {
                        first = false;
                        fString += filterString(f, true);
                    } else
                        fString += filterString(f);
                    delete f;
                }
            }
        }
        auto fdi(new FiltersDialogItem(fname, fString, this));
        items.append(fdi);
        ui->filtersLayout->addWidget(fdi, 0, Qt::AlignTop);
        connect(fdi, &FiltersDialogItem::Filter, this, &FilterBar::slotLoadFilter);
        connect(fdi, &FiltersDialogItem::Remove, this, &FilterBar::slotRemoveFilter);
    }
}

void FilterBar::slotTagSelect(const QModelIndex &index)
{
    QString tag(tagListModel->data(index, TagListModel::TagString).toString());
    QString key(tagListModel->data(index, TagListModel::TagType).toString());

    //	int selCount(ui->tagsView->selectionModel()->selectedIndexes().count());
    //	if(selCount == 1)
    {
        for (auto *f : std::as_const(filters)) {
            if (f->filter()->data(FilterTag::Tag).toString() == tag)
                return;
        }
    }

    int andTag(ui->tagsView->getAndKey());
    if (0 == andTag) {
        slotClearFilter();
        tagListModel->clearCurrents();
    }
    auto ft(new FilterTag);
    ft->setData(FilterData::Text, tag);
    ft->setData(FilterTag::Key, key);
    ft->setData(FilterTag::Tag, tag);
    if (1 == andTag) {
        ft->setData(FilterData::And, true);
        ft->setData(FilterData::Or, false);
    } else if (2 == andTag) {
        ft->setData(FilterData::And, true);
        ft->setData(FilterData::Or, false);
        ft->setData(FilterData::Not, true);
    }
    tagListModel->addToCurrents(tag);
    addFilterItem(ft);
}

void FilterBar::slotTagEdit(const QModelIndex &index)
{
    // "Activated" is not a tag of the database, there is nothing to rename
    if (tagListModel->data(index, TagListModel::TagType).toString() != QString("TAG"))
        return;
    QString tag(tagListModel->data(index, TagListModel::TagString).toString());
    bool ok;
    QString newTag(QInputDialog::getText(this,
                                         i18nc("@title:window", "Fontmatrix - edit tag"),
                                         i18nc("@label:textbox", "Edit tag: ") + tag,
                                         QLineEdit::Normal,
                                         QString(),
                                         &ok));
    if (!ok || newTag.isEmpty() || newTag == tag)
        return;

    // A filter on this tag would go on asking for the old name and find
    // nothing. The fonts are the same, so nothing has to be filtered again.
    for (auto *f : std::as_const(filters)) {
        FilterData *fd(f->filter());
        if (fd->type() == QString("Tag") && fd->data(FilterTag::Key).toString() == QString("TAG") && fd->data(FilterTag::Tag).toString() == tag) {
            fd->setData(FilterTag::Tag, newTag);
            fd->setData(FilterData::Text, newTag);
            f->updateText();
        }
    }
    // before editTag(): it resets the model, which reads the current tags
    tagListModel->renameCurrent(tag, newTag);
    FMFontDb::DB()->editTag(tag, newTag);
}

void FilterBar::slotPanoFilter()
{
    QMap<int, QList<int>> pv(ui->panoseWidget->getFilter());
    const QMap<FontStrings::PanoseKey, QMap<int, QString>> &ps(FontStrings::Panose());
    for (const auto pvKeys = pv.keys(); const auto &k : pvKeys) {
        for (const auto range = pv[k]; const auto &v : range) {
            auto pk(static_cast<FontStrings::PanoseKey>(k));
            QString text(FontStrings::PanoseKeyName(pk) + QString(" : ") + ps.value(pk).value(v));
            auto fp(new FilterPanose);
            fp->setData(FilterData::Text, text);
            fp->setData(FilterPanose::Param, k);
            fp->setData(FilterPanose::Value, v);
            addFilterItem(fp);
        }
    }
}

void FilterBar::metaSelectField(int idx)
{
    //	metaFieldKey = action->data().toInt();
    //	ui->metaFieldLabel->setText(FontStrings::Names().value(FMFontDb::InfoItem(metaFieldKey)));
    metaFieldKey = ui->fieldCombo->itemData(idx).toInt();
}

void FilterBar::slotClearFilter()
{
    removeAllFilters();
    tagListModel->clearCurrents();
    Q_EMIT filterChanged();
}

void FilterBar::slotSaveFilter()
{
    if (filters.isEmpty())
        return;

    bool ok;
    QString fname = QInputDialog::getText(this,
                                          i18nc("@title:window", "Fontmatrix - Filter name"),
                                          i18nc("@label:textbox", "Filter name:"),
                                          QLineEdit::Normal,
                                          QString(""),
                                          &ok);
    if (!ok || fname.isEmpty())
        return;

    slotSaveFilter(fname);
}

void FilterBar::slotSaveFilter(const QString &fname)
{
    if (filters.isEmpty() || fname.isEmpty())
        return;

    QDir fdir(FMPaths::FiltersDir());
    if (!fdir.exists(fname))
        fdir.mkdir(fname);
    fdir.cd(fname);
    for (int i(0); i < filters.count(); ++i) {
        QFile f(fdir.absoluteFilePath(QString("%1-%2").arg(i, 3, 10, QChar('0')).arg(filters.at(i)->filter()->type())));
        if (f.open(QIODevice::WriteOnly)) {
            f.write(filters.at(i)->filter()->toByteArray());
            f.close();
        }
    }
    loadFilters();
}

void FilterBar::slotLoadFilter(const QString &fname)
{
    removeAllFilters();
    QDir fdir(FMPaths::FiltersDir() + fname);
    QStringList flist(fdir.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::Name));
    for (const auto &fn : std::as_const(flist)) {
        QStringList l(fn.split(QString("-")));
        if (l.count() == 2) {
            QString type(l.at(1));
            QFile file(fdir.absoluteFilePath(fn));
            if (file.open(QIODevice::ReadOnly)) {
                FilterData *f = nullptr;
                if (type == QString("Meta")) {
                    f = new FilterMeta;
                } else if (type == QString("Panose")) {
                    f = new FilterPanose;
                } else if (type == QString("Tag")) {
                    f = new FilterTag;
                } else if (type == QString("Lang")) {
                    f = new FilterLang;
                } else if (type == QString("License")) {
                    f = new FilterLicense;
                } else if (type == QString("Duplicate")) {
                    f = new FilterDuplicate;
                } else if (type == QString("Kind")) {
                    f = new FilterKind;
                }
                if (!f)
                    continue;
                f->fromByteArray(file.readAll());
                addFilterItem(f);
            }
        }
    }
    processFilters();
}

void FilterBar::slotRemoveFilter(const QString &fname)
{
    if (fname.isEmpty())
        return;

    QDir fdir(FMPaths::FiltersDir());
    if (fdir.exists(fname)) {
        fdir.cd(fname);
        QStringList flist(fdir.entryList(QDir::NoDotAndDotDot | QDir::Files));
        for (const auto &fn : std::as_const(flist)) {
            fdir.remove(fn);
        }
        fdir.cd(FMPaths::FiltersDir());
        fdir.rmdir(fname);
    } else {
        qCDebug(FONTMATRIX_LOG) << "Directory does not exist:" << fdir.absolutePath() << fname;
    }
    loadFilters();
}

void FilterBar::filtersDialog()
{
    auto fd(new FiltersDialog(filters, this));
    connect(fd, &FiltersDialog::Filter, this, &FilterBar::slotLoadFilter);
    connect(fd, &FiltersDialog::AddFilter, this, qOverload<const QString &>(&FilterBar::slotSaveFilter));
    connect(fd, &FiltersDialog::RemoveFilter, this, &FilterBar::slotRemoveFilter);
    fd->exec();
}

void FilterBar::slotToggleTags(bool t)
{
    if (t)
        ui->tagsBox->show();
    else
        ui->tagsBox->hide();
}

void FilterBar::slotToggleMeta(bool t)
{
    if (t)
        ui->metadataBox->show();
    else
        ui->metadataBox->hide();
}

void FilterBar::slotTogglePano(bool t)
{
    if (t)
        ui->panoseBox->show();
    else
        ui->panoseBox->hide();
}

void FilterBar::slotToggleFilter(bool t)
{
    if (t)
        ui->filtersBox->show();
    else
        ui->filtersBox->hide();
}

void FilterBar::slotToggleLanguages(bool t)
{
    // the section is not there at all when nothing can tell the coverage
    if (t && FMLangCoverage::isAvailable()) {
        fillLanguages();
        ui->languagesBox->show();
    } else {
        ui->languagesBox->hide();
    }
}

void FilterBar::slotToggleDuplicates(bool t)
{
    if (t) {
        fillDuplicates();
        ui->duplicatesBox->show();
    } else {
        ui->duplicatesBox->hide();
    }
}

void FilterBar::slotToggleKinds(bool t)
{
    if (t) {
        fillKinds();
        ui->kindBox->show();
    } else {
        ui->kindBox->hide();
    }
}

void FilterBar::slotToggleLicense(bool t)
{
    if (t) {
        fillLicenses();
        ui->licenseBox->show();
    } else {
        ui->licenseBox->hide();
    }
}

void FilterBar::fillLanguages()
{
    if (languagesFilled || !FMLangCoverage::isAvailable())
        return;
    languagesFilled = true;
    ui->languagesCombo->clear();
    ui->languagesCombo->addItem(i18nc("@item:inlistbox no language picked yet, in the list of the languages a font can set", "Select language"), QString());
    const QStringList languages(FMLangCoverage::languages());
    for (const QString &language : languages) {
        ui->languagesCombo->addItem(FMLangCoverage::name(language), language);
        const QString native(FMLangCoverage::nativeName(language));
        if (!native.isEmpty())
            ui->languagesCombo->setItemData(ui->languagesCombo->count() - 1, native, Qt::ToolTipRole);
    }
}

void FilterBar::fillLicenses()
{
    if (licensesFilled)
        return;
    licensesFilled = true;
    ui->licenseCombo->clear();
    ui->licenseCombo->addItem(i18nc("@item:inlistbox no licence picked yet, in the list of the licences of the fonts", "Select license"), -1);
    const QList<FMLicense::Family> families(FMLicense::families());
    for (const FMLicense::Family family : families)
        ui->licenseCombo->addItem(FMLicense::name(family), int(family));
}

void FilterBar::fillDuplicates()
{
    if (duplicatesFilled)
        return;
    duplicatesFilled = true;
    ui->duplicatesCombo->clear();
    ui->duplicatesCombo->addItem(i18nc("@item:inlistbox nothing picked yet, in the list of the kinds of duplicate", "Select kind"), -1);
    // the files are read here, so the count is known before anything is picked
    typotek::getInstance()->showStatusMessage(i18nc("@info:status", "Looking for the fonts the collection holds more than once..."));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const int identical(FMDuplicates::count(FMDuplicates::IdenticalFiles));
    const int same(FMDuplicates::count(FMDuplicates::SameFont));
    QApplication::restoreOverrideCursor();
    typotek::getInstance()->showStatusMessage(QString());
    ui->duplicatesCombo->addItem(
        i18ncp("@item:inlistbox files that are byte for byte the same, %1 how many fonts", "Identical file (%1 font)", "Identical files (%1 fonts)", identical),
        int(FMDuplicates::IdenticalFiles));
    ui->duplicatesCombo->addItem(i18ncp("@item:inlistbox one family, style and version in files that differ, %1 how many fonts",
                                        "Same font, other file (%1 font)",
                                        "Same font, other file (%1 fonts)",
                                        same),
                                 int(FMDuplicates::SameFont));
}

void FilterBar::slotDuplicateFilter(int index)
{
    const int kind(ui->duplicatesCombo->itemData(index).toInt());
    if (kind < 0)
        return;
    auto fd(new FilterDuplicate);
    const QString what(kind == int(FMDuplicates::IdenticalFiles)
                           ? i18nc("@item a filter on the fonts whose files are the same", "Duplicates: identical files")
                           : i18nc("@item a filter on the fonts of one family, style and version", "Duplicates: same font, other file"));
    fd->setData(FilterData::Text, what);
    fd->setData(FilterDuplicate::Kind, kind);
    addFilterItem(fd);
    ui->duplicatesCombo->setCurrentIndex(0);
}

void FilterBar::fillKinds()
{
    if (kindsFilled)
        return;
    kindsFilled = true;
    ui->kindCombo->clear();
    ui->kindCombo->addItem(i18nc("@item:inlistbox nothing picked yet, in the list of variable and colour fonts", "Select technology"), -1);
    // every face is opened once here, so the count is known before anything is picked
    typotek::getInstance()->showStatusMessage(i18nc("@info:status", "Looking for variable and colour fonts..."));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const int variable(FMFontKinds::count(FMFontKinds::Variable));
    const int colour(FMFontKinds::count(FMFontKinds::Colour));
    QApplication::restoreOverrideCursor();
    typotek::getInstance()->showStatusMessage(QString());
    ui->kindCombo->addItem(i18ncp("@item:inlistbox fonts with variation axes, %1 how many", "Variable font (%1)", "Variable fonts (%1)", variable),
                           int(FMFontKinds::Variable));
    ui->kindCombo->addItem(i18ncp("@item:inlistbox fonts with colour glyphs, %1 how many", "Colour font (%1)", "Colour fonts (%1)", colour),
                           int(FMFontKinds::Colour));
}

void FilterBar::slotKindFilter(int index)
{
    const int kind(ui->kindCombo->itemData(index).toInt());
    if (kind < 0)
        return;
    auto fk(new FilterKind);
    fk->setData(FilterData::Text,
                kind == int(FMFontKinds::Variable) ? i18nc("@item a filter on the variable fonts", "Variable fonts")
                                                   : i18nc("@item a filter on the colour fonts", "Colour fonts"));
    fk->setData(FilterKind::Kind, kind);
    addFilterItem(fk);
    ui->kindCombo->setCurrentIndex(0);
}

void FilterBar::slotLangFilter(int index)
{
    const QString language(ui->languagesCombo->itemData(index).toString());
    if (language.isEmpty())
        return;
    auto fl(new FilterLang);
    fl->setData(FilterData::Text, i18nc("@item a filter on the language a font can set", "Language: %1", FMLangCoverage::name(language)));
    fl->setData(FilterLang::Language, language);
    addFilterItem(fl);
    ui->languagesCombo->setCurrentIndex(0);
}

void FilterBar::slotLicenseFilter(int index)
{
    const int family(ui->licenseCombo->itemData(index).toInt());
    if (family < 0)
        return;
    auto fl(new FilterLicense);
    fl->setData(FilterData::Text, i18nc("@item a filter on the licence of a font", "License: %1", FMLicense::name(static_cast<FMLicense::Family>(family))));
    fl->setData(FilterLicense::License, family);
    addFilterItem(fl);
    ui->licenseCombo->setCurrentIndex(0);
}

#include "moc_filterbar.cpp"
