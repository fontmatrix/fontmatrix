/*
    SPDX-FileCopyrightText: 2011 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TAGSWIDGET_LISTMODEL_H
#define TAGSWIDGET_LISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QVariant>

class FontItem;

class TagsWidget_ListModel : public QAbstractListModel
{
    Q_OBJECT

    QList<FontItem *> fonts;
    QStringList tags;
    const QString newTagString;

private Q_SLOTS:
    void updateTags();

public:
    explicit TagsWidget_ListModel(QObject *parent);
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setFonts(const QList<FontItem *> &flist);
    QModelIndex addTag();
};

#endif // TAGSWIDGET_LISTMODEL_H
