/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "floatingwidgetsregister.h"

#include "floatingwidget.h"

FloatingWidgetsRegister *FloatingWidgetsRegister::instance = nullptr;

FloatingWidgetsRegister::FloatingWidgetsRegister() = default;

FloatingWidgetsRegister *FloatingWidgetsRegister::that()
{
    if (instance == nullptr)
        instance = new FloatingWidgetsRegister;
    return instance;
}

void FloatingWidgetsRegister::Register(FloatingWidget *f, const QString &fid, const QString &typ)
{
    FloatingWidgetsRegister *fwr(that());
    fwr->fwMap[typ][fid] = f;
}

FloatingWidget *FloatingWidgetsRegister::Widget(const QString &fid, const QString &typ)
{
    FloatingWidgetsRegister *fwr(that());
    if (fwr->fwMap.contains(typ)) {
        if (fwr->fwMap.value(typ).contains(fid)) {
            if (fwr->fwMap[typ][fid].isNull())
                fwr->fwMap[typ].remove(fid);
            else
                return fwr->fwMap[typ][fid];
        }
    }
    return nullptr;
}

QList<FloatingWidget *> FloatingWidgetsRegister::AllWidgets()
{
    QList<FloatingWidget *> ret;
    FloatingWidgetsRegister *fwr(that());
    ret.clear();
    for (const auto keysList = fwr->fwMap.keys(); const auto &t : keysList) {
        for (const auto loopKeys = fwr->fwMap.value(t).keys(); const auto &f : loopKeys) {
            if (fwr->fwMap[t][f].isNull())
                fwr->fwMap[t].remove(f);
            else
                ret << fwr->fwMap[t][f].data();
        }
    }
    return ret;
}
