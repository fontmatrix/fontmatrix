/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
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
#ifndef DATALOADER_H
#define DATALOADER_H

#include <QMap>
#include <QString>

/**
	@author Pierre Marchand <pierre@oep-h.com>
*/
class DataLoader
{
	
	QMap<QString, QMap<QString,QString> > sm;
	QMap<QString,QString> pm;

	void load();
public:
	DataLoader();
	~DataLoader(){}

	bool update(const QString& name, const QString& sample);
	bool remove(const QString& name);
	void reload();


	const QMap<QString, QMap<QString,QString> >& systemSamples()const{return sm;}
	const QMap<QString,QString>& userSamples()const{return pm;}

};

#endif
