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

void Shortcuts::add(QAction *a, const QString &description)
{
	if (actions.contains(a->text()))
		return;

	QString settingsKey = QString("ActionShortcut-%1").arg(a->text());
	if (settings.contains(settingsKey))
		a->setShortcut(QKeySequence(settings.value(settingsKey).toString()));
	actions[a->text()] = a;
}

QList<QAction*> Shortcuts::getActions()
{
	return actions.values();
}

Shortcuts::~Shortcuts()
{

}
