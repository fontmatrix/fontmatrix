/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FLOATINGWIDGETSREGISTER_H
#define FLOATINGWIDGETSREGISTER_H

// #include <QObject>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>

class FloatingWidget;

class FloatingWidgetsRegister
{
    static FloatingWidgetsRegister *instance;
    FloatingWidgetsRegister();
    ~FloatingWidgetsRegister() = default;
    static FloatingWidgetsRegister *that();

public:
    static void Register(FloatingWidget *f, const QString &fid, const QString &typ);
    static FloatingWidget *Widget(const QString &fid, const QString &typ);
    static QList<FloatingWidget *> AllWidgets();

private:
    QMap<QString, QMap<QString, QPointer<FloatingWidget>>> fwMap; // map[ type , [ fontID , pointer ] ]
};

#endif // FLOATINGWIDGETSREGISTER_H
