/*
    SPDX-FileCopyrightText: 2008 Riku Leino <riku@scribus.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shortcuts.h"
#include "fmconfig.h"

#include <QAction>

Shortcuts *Shortcuts::instance = nullptr;

Shortcuts::Shortcuts() = default;

Shortcuts *Shortcuts::getInstance()
{
    if (instance == nullptr)
        instance = new Shortcuts();

    return instance;
}

void Shortcuts::add(QAction *a)
{
    if (actions.contains(cleanName(a)))
        return;

    QString key = settingsKey(a);
    if (FMConfig::contains(key))
        a->setShortcut(QKeySequence(FMConfig::value(key).toString()));
    actions[cleanName(a)] = a;
}

QList<QAction *> Shortcuts::getActions()
{
    return actions.values();
}

QString Shortcuts::settingsKey(QAction *action)
{
    return QStringLiteral("ActionShortcut/%1").arg(cleanName(action));
}

QString Shortcuts::cleanName(QAction *action)
{
    return cleanName(action->text());
}

QString Shortcuts::cleanName(const QString &s)
{
    QString h = s;
    return h.remove(QStringLiteral("&"));
}

QString Shortcuts::isReserved(const QString &shortcut, const QString &actionText)
{
    QString isTaken;
    if (actions.contains(cleanName(actionText))) {
        QList<QAction *> alist = actions.values();
        for (auto *act : std::as_const(alist)) {
            if (act->shortcut() == shortcut) {
                isTaken = act->text();
                break;
            }
        }
    }
    return isTaken;
}

void Shortcuts::setShortcut(const QString &shortcut, const QString &actionText)
{
    if (actions.contains(cleanName(actionText))) {
        actions.value(cleanName(actionText))->setShortcut(shortcut);
        FMConfig::setValue(settingsKey(actions.value(cleanName(actionText))), shortcut);
    }
}

void Shortcuts::clearShortcut(const QString &actionText)
{
    if (actions.contains(cleanName(actionText))) {
        actions.value(cleanName(actionText))->setShortcut(QKeySequence());
        FMConfig::setValue(settingsKey(actions.value(cleanName(actionText))), QString());
    }
}

Shortcuts::~Shortcuts()
{
}

#include "moc_shortcuts.cpp"
