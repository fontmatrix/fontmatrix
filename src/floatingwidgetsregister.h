/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
 *   pierre@oep-h.com   *
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

#ifndef FLOATINGWIDGETSREGISTER_H
#define FLOATINGWIDGETSREGISTER_H

//#include <QObject>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QList>

class FloatingWidget;

class FloatingWidgetsRegister
{
	static FloatingWidgetsRegister * instance;
	FloatingWidgetsRegister();
	~FloatingWidgetsRegister(){}
	static FloatingWidgetsRegister* that();
public:
	static void Register(FloatingWidget* f, const QString& fid, const QString& typ);
	static FloatingWidget* Widget(const QString& fid, const QString& typ);
	static QList<FloatingWidget*> AllWidgets();

private:
	QMap< QString, QMap < QString, QPointer<FloatingWidget> > > fwMap; // map[ type , [ fontID , pointer ] ]


};

#endif // FLOATINGWIDGETSREGISTER_H
