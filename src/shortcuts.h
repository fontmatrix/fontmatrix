/*
    SPDX-FileCopyrightText: 2008 Riku Leino <riku@scribus.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QMap>
#include <QObject>
#include <QString>

class QAction;

class Shortcuts : public QObject
{
    Q_OBJECT
public:
    ~Shortcuts() override;

    static Shortcuts *getInstance();

    void add(QAction *a);

    QList<QAction *> getActions();

    /* returns QString::null if it's not reserved or the action name
      the shortcut belongs if it's already taken. */
    QString isReserved(const QString &shortcut, const QString &actionText);

    void setShortcut(const QString &shortcut, const QString &actionText);

    void clearShortcut(const QString &actionText);

    QString cleanName(QAction *action); // without & chars
    QString cleanName(const QString &s); // same with a string

private:
    QMap<QString, QAction *> actions;

    static Shortcuts *instance;

    QString settingsKey(QAction *action);

protected:
    Shortcuts();
};

#endif
