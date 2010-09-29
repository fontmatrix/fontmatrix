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

#ifndef FMFREETYPELIB_H
#define FMFREETYPELIB_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <QObject>
#include <QMap>
#include <QMutex>

class FMFreetypeLib : public QObject
{
	Q_OBJECT

	static FMFreetypeLib * instance;
	static FMFreetypeLib * that();
	explicit FMFreetypeLib(QObject *parent = 0);

	QMap<QThread *, FT_Library> libraries;
	QMutex *mutex;

	class FTLibFactory : public QObject
	{
	public:
		FT_Library createLib()
		{
			FT_Library theLibrary;
			FT_Init_FreeType ( &theLibrary );
			return theLibrary;
		}
	};

public:
	static FT_Library lib(QThread * t);

private slots:
	void releaseLibrary();

};

#endif // FMFREETYPELIB_H
