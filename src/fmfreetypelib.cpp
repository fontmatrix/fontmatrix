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

#include "fmfreetypelib.h"

#include <QThread>
#include <QMutexLocker>
#include <QDebug>

FMFreetypeLib *FMFreetypeLib::instance = 0;

FMFreetypeLib::FMFreetypeLib(QObject *parent) :
    QObject(parent)
{
	FT_Library theLibrary;
	FT_Init_FreeType ( &theLibrary );
	libraries.insert(thread(), theLibrary);
	qDebug()<<"FT_Library"<<theLibrary<<thread();
	mutex = new QMutex;
}

FMFreetypeLib * FMFreetypeLib::that()
{
	if(0 == instance)
		instance = new FMFreetypeLib;
	return instance;
}

FT_Library FMFreetypeLib::lib(QThread *t)
{
//	return that()->libraries.value(that()->thread());
	QMutexLocker(that()->mutex);
	if(that()->libraries.contains(t))
		return that()->libraries.value(t);

	FTLibFactory ff;
	ff.moveToThread(t);
	that()->libraries.insert(t, ff.createLib());
	connect(t, SIGNAL(terminated()), that(), SLOT(releaseLibrary()));
	return that()->libraries.value(t);
}

void FMFreetypeLib::releaseLibrary()
{
	if(sender())
	{
		QThread *t(reinterpret_cast<QThread*>(sender()));
		if(t && libraries.contains(t))
		{
			FT_Done_FreeType(libraries[t]);
			libraries.remove(t);
		}
	}
}
