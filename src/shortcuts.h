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

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QSettings>
#include <QMap>

class QAction;

class Shortcuts : public QObject
{
	Q_OBJECT
public:
	~Shortcuts();

	static Shortcuts* getInstance();

	void add(QAction *a);

	QList<QAction*> getActions();

private:
	QSettings settings;

	QMap<QString, QAction*> actions;

	static Shortcuts* instance;

	QString settingsKey(QAction *action);

protected:
	Shortcuts();
};

#endif
