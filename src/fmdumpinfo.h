/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMDUMPINFO_H
#define FMDUMPINFO_H

#include <QMap>
#include <QString>
#include <QStringList>

class FontItem;

class FMDumpInfo
{
public:
    explicit FMDumpInfo(FontItem *font, const QString &model = QString());
    ~FMDumpInfo();

    QStringList infos()
    {
        return m_info.keys();
    }
    QString info(const QString &k)
    {
        return m_info.value(k);
    }
    void setModel(const QString &model)
    {
        m_model = model;
    }
    bool dumpInfo(const QString &filepath);

private:
    FontItem *m_font = nullptr;
    QString m_model;

    QMap<QString, QString> m_info;
};

#endif // FMDUMPINFO_H
