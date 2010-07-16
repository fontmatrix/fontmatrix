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

#include "filtertag.h"
#include "filteritem.h"
#include "fmfontdb.h"

FilterTag::FilterTag():
		FilterData()
{
}

QString FilterTag::type() const
{
	return QString("Tag");
}

void FilterTag::operate()
{
	QString key(vData.value(Key).toString());
	QString tag(vData.value(Tag).toString());

	if(key == "TAG") // regular tag
	{
		operateFilter( FMFontDb::DB()->Fonts(tag, FMFontDb::Tags ) );
	}
	else if(key == "ALL_ACTIVATED")
	{
		operateFilter( FMFontDb::DB()->Fonts(1, FMFontDb::Activation ) );
	}
}
