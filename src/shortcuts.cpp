/***************************************************************************
 *   Copyright (C) 2008 by Riku Leino                                      *
 *   riku@scribus.info                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "shortcuts.h"

#include <QAction>

Shortcuts* Shortcuts::instance = 0;

Shortcuts::Shortcuts()
{

}

Shortcuts* Shortcuts::getInstance()
{
	if (instance == 0)
		instance = new Shortcuts();

	return instance;
}

void Shortcuts::add(QAction *a)
{
	if (actions.contains(a->text()))
		return;

	QString key = settingsKey(a);
	if (settings.contains(key))
		a->setShortcut(QKeySequence(settings.value(key).toString()));
	actions[a->text()] = a;
}

QList<QAction*> Shortcuts::getActions()
{
	return actions.values();
}

QString Shortcuts::settingsKey(QAction *action)
{
	return QString("ActionShortcut-%1").arg(action->text());
}

QString Shortcuts::isReserved(const QString &shortcut, const QString &actionText)
{
	QString isTaken = QString::null;
	if (actions.contains(actionText)) {
		QList<QAction*> alist = actions.values();
		foreach(QAction *act, alist) {
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
	if (actions.contains(actionText)) {
		actions[actionText]->setShortcut(shortcut);
		settings.setValue(settingsKey(actions[actionText]), shortcut);
	}
}

void Shortcuts::clearShortcut(const QString &actionText)
{
	if (actions.contains(actionText)) {
		actions[actionText]->setShortcut(QString(""));
		settings.setValue(settingsKey(actions[actionText]), QString(""));
	}
}

Shortcuts::~Shortcuts()
{

}
