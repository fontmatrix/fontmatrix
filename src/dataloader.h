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

#include <QFile>
#include <QStringList>

#include "fontitem.h"

class typotek;

/**
	@author Pierre Marchand <pierre@oep-h.com>
*/
class DataLoader
{
		
	public:
		DataLoader ( QFile *file );

		~DataLoader();
		void load();
		QStringList fontList() const { return m_fontList; }
		QMap<  QString,FontLocalInfo > fastList() const { return m_fastList; }
	private :
		QFile *m_file;
		typotek *m_typo;
		QStringList m_fontList;
		QMap<  QString,FontLocalInfo > m_fastList;


};

#endif
