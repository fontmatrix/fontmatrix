/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANOSEMODEL_H
#define PANOSEMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QString>

class PanoseAttributeModel : public QAbstractListModel
{
public:
    explicit PanoseAttributeModel(QObject *parent);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

private:
    QStringList m_names;
    QList<QIcon> m_icons;
};

class PanoseValueModel : public QAbstractListModel
{
public:
    explicit PanoseValueModel(QObject *parent);

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    void setCat(const int &cat);

private:
    int m_cat;
    QMap<int, QList<QIcon>> m_icons;
    QMap<int, QStringList> m_names;
};

#endif // PANOSEMODEL_H
