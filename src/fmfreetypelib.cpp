/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmfreetypelib.h"
#include "fontmatrix_debug.h"

#include <QDebug>
#include <QMutexLocker>
#include <QThread>

FMFreetypeLib *FMFreetypeLib::instance = nullptr;

FMFreetypeLib::FMFreetypeLib(QObject *parent)
    : QObject(parent)
{
    FT_Library theLibrary;
    FT_Init_FreeType(&theLibrary);
    libraries.insert(thread(), theLibrary);
    qCDebug(FONTMATRIX_LOG) << "FT_Library" << theLibrary << thread();
    mutex = new QMutex;
}

FMFreetypeLib *FMFreetypeLib::that()
{
    if (nullptr == instance)
        instance = new FMFreetypeLib;
    return instance;
}

FT_Library FMFreetypeLib::lib(QThread *t)
{
    //	return that()->libraries.value(that()->thread());
    QMutexLocker lock(that()->mutex);
    if (that()->libraries.contains(t))
        return that()->libraries.value(t);

    FTLibFactory ff;
    ff.moveToThread(t);
    that()->libraries.insert(t, ff.createLib());
    connect(t, &QThread::finished, that(), &FMFreetypeLib::releaseLibrary);
    return that()->libraries.value(t);
}

void FMFreetypeLib::releaseLibrary()
{
    if (sender()) {
        auto t(reinterpret_cast<QThread *>(sender()));
        if (t && libraries.contains(t)) {
            FT_Done_FreeType(libraries.value(t));
            libraries.remove(t);
        }
    }
}

#include "moc_fmfreetypelib.cpp"
