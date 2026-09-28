/*
    SPDX-FileCopyrightText: 2011 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tagswidget_listmodel.h"

#include "fmfontdb.h"
#include "fontitem.h"

#include <KLocalizedString>
#include <QFont>
#include <QModelIndex>
#include <QStringList>

TagsWidget_ListModel::TagsWidget_ListModel(QObject *parent)
    : QAbstractListModel(parent)
    , newTagString(i18nc("@item:inlistbox default name of a new tag", "New Tag"))
{
    connect(FMFontDb::DB(), &FMFontDb::tagsChanged, this, &TagsWidget_ListModel::updateTags);
}

void TagsWidget_ListModel::updateTags()
{
    Q_EMIT dataChanged(index(0), index(rowCount() - 1));
}

int TagsWidget_ListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return FMFontDb::DB()->getTags().count();
}

int TagsWidget_ListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 1;
}

QVariant TagsWidget_ListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return QVariant();
    // getTags() is cached and sorted
    const QString tag(FMFontDb::DB()->getTags().value(index.row()));
    if (role == Qt::DisplayRole) {
        return tag;
    } else if (role == Qt::EditRole)
        return tag;
    else if (role == Qt::CheckStateRole) {
        if (tags.contains(tag))
            return Qt::Checked;
        return Qt::Unchecked;
    } else if (role == Qt::FontRole) {
        QFont dfont;
        if (tags.contains(tag)) {
            dfont.setBold(true);
            return dfont;
        }
        return dfont;
    }

    return QVariant();
}

bool TagsWidget_ListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (value.toString().isEmpty())
        return false;

    if (role == Qt::CheckStateRole) {
        const QStringList tl_tmp = FMFontDb::DB()->getTags();
        QString tag(tl_tmp.at(index.row()));
        if (Qt::CheckState(value.toInt()) == Qt::Checked) {
            if (!tags.contains(tag)) {
                FMFontDb::DB()->TransactionBegin();
                for (auto *f : std::as_const(fonts))
                    f->addTag(tag);
                FMFontDb::DB()->TransactionEnd();
                tags.append(tag);
                Q_EMIT dataChanged(index, index);
                return true;
            } else
                return false;
        } else if (Qt::CheckState(value.toInt()) == Qt::Unchecked) {
            if (tags.contains(tag)) {
                FMFontDb::DB()->TransactionBegin();
                for (auto *f : std::as_const(fonts))
                    FMFontDb::DB()->removeTag(f->path(), tag);
                FMFontDb::DB()->TransactionEnd();
                tags.removeAll(tag);
                Q_EMIT dataChanged(index, index);
                return true;
            } else
                return false;
        }
    } else if (role == Qt::EditRole || role == Qt::DisplayRole) {
        const QStringList tl_tmp = FMFontDb::DB()->getTags();
        if (value.toString() == tl_tmp.at(index.row()))
            return false;
        FMFontDb::DB()->editTag(tl_tmp.at(index.row()), value.toString());
        Q_EMIT dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags TagsWidget_ListModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void TagsWidget_ListModel::setFonts(const QList<FontItem *> &flist)
{
    fonts.clear();
    fonts = flist;
    if (fonts.count() > 0)
        tags = fonts.first()->tags();
    updateTags();
}

QModelIndex TagsWidget_ListModel::addTag()
{
    if (FMFontDb::DB()->getTags().contains(newTagString))
        return QModelIndex();

    FMFontDb::DB()->addTagToDB(newTagString);
    updateTags();

    const QStringList tl_tmp = FMFontDb::DB()->getTags();

    for (int i(0); i < tl_tmp.count(); ++i) {
        if (tl_tmp.at(i) == newTagString)
            return index(i);
    }

    return QModelIndex();
}

#include "moc_tagswidget_listmodel.cpp"
