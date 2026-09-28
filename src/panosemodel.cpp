/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panosemodel.h"
#include "fmfontstrings.h"
#include "fmpaths.h"
#include "parallelcoor.h"

#include "fmconfig.h"
#include <QDir>
#include <QFile>

/// ATTRIBUTE MODEL

PanoseAttributeModel::PanoseAttributeModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QMap<FontStrings::PanoseKey, QMap<int, QString>> &p(FontStrings::Panose());
    QString defaultDir(FMPaths::ResourcesDir() + QLatin1String("Panose/Icons"));
    QString pDir(FMConfig::value(QStringLiteral("Panose/IconDir"), defaultDir).toString() + QDir::separator());
    for (const auto pKeys = p.keys(); const auto &k : pKeys) {
        QString fn(pDir + QString::number(k) + QDir::separator() + QLatin1String("attribute.png"));
        if (QFile::exists(fn))
            m_icons << QIcon(fn);
        else
            m_icons << QIcon();
        m_names << FontStrings::PanoseKeyName(k);
    }
}

QVariant PanoseAttributeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (Qt::DisplayRole == role) {
        return m_names.at(index.row());
    } else if (Qt::DecorationRole == role) {
        return m_icons.at(index.row());
    }

    return QVariant();
}

int PanoseAttributeModel::rowCount(const QModelIndex &) const
{
    return m_icons.count();
}

/// END OF ATTRIBUTE MODEL

/// VALUE MODEL

PanoseValueModel::PanoseValueModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QMap<FontStrings::PanoseKey, QMap<int, QString>> &p(FontStrings::Panose());
    QString defaultDir(FMPaths::ResourcesDir() + QLatin1String("Panose/Icons"));
    QString pDir(FMConfig::value(QStringLiteral("Panose/IconDir"), defaultDir).toString() + QDir::separator());

    for (const auto pKeysList = p.keys(); const auto &k : pKeysList) {
        for (const auto loopPKeys = p[k].keys(); const auto &v : loopPKeys) {
            if (v > 1) // We do not want "Any" and "No Fit"
            {
                QString fn(pDir + QString::number(k) + QDir::separator() + QString::number(v) + QLatin1String(".png"));
                if (QFile::exists(fn))
                    m_icons[k] << QIcon(fn);
                else
                    m_icons[k] << QIcon();
                m_names[k] << p.value(k).value(v);
            }
        }
    }
    m_cat = FontStrings::firstPanoseKey();
}

void PanoseValueModel::setCat(const int &cat)
{
    if (cat != m_cat) {
        m_cat = cat;
        Q_EMIT layoutChanged();
    }
}

int PanoseValueModel::rowCount(const QModelIndex &) const
{
    return m_names[m_cat].count();
}

QVariant PanoseValueModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (Qt::DisplayRole == role) {
        if (m_icons[m_cat].at(index.row()).isNull())
            return m_names[m_cat].at(index.row());
        return QString();
    } else if (Qt::DecorationRole == role) {
        return m_icons[m_cat].at(index.row());
    } else if (Qt::ToolTipRole == role) {
        return m_names[m_cat].at(index.row());
    }
    return QVariant();
}

/// END OF VALUE MODEL
