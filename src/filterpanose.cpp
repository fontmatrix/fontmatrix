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

#include "filterpanose.h"
#include "fmfontdb.h"

FilterPanose::FilterPanose():
		FilterData()
{
}

QString FilterPanose::type() const
{
	return QString("Panose");
}


void FilterPanose::operate()
{
	QList<FontDBResult> dbresult( FMFontDb::DB()->getValues( FMFontDb::Panose ) );
	QList<FontItem*> fil;
	int paramIdx(vData[Param].toInt());
	int val(vData[Value].toInt());
	int fv(0);
	for(int i(0); i < dbresult.count() ; ++i)
	{
		QStringList pl(dbresult[i].second.split(":"));
		fv = pl[paramIdx].toInt();
		if(fv == val)
			fil << dbresult[i].first;
	}
	operateFilter(fil);
}
