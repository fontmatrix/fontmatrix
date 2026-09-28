/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMFREETYPELIB_H
#define FMFREETYPELIB_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <QMap>
#include <QMutex>
#include <QObject>

class FMFreetypeLib : public QObject
{
    Q_OBJECT

    static FMFreetypeLib *instance;
    static FMFreetypeLib *that();
    explicit FMFreetypeLib(QObject *parent = nullptr);

    QMap<QThread *, FT_Library> libraries;
    QMutex *mutex = nullptr;

    class FTLibFactory : public QObject
    {
    public:
        FT_Library createLib()
        {
            FT_Library theLibrary;
            FT_Init_FreeType(&theLibrary);
            return theLibrary;
        }
    };

public:
    static FT_Library lib(QThread *t);

private Q_SLOTS:
    void releaseLibrary();
};

#endif // FMFREETYPELIB_H
