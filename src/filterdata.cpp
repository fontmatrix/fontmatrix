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

#include "filterdata.h"
#include "fmfontdb.h"
#include "filteritem.h"

FilterData::FilterData()
{
	vData.clear();
	// default operation
	vData.insert(Replace, false);
	vData.insert(Or, true);
	vData.insert(And, false);
	vData.insert(Not, false);
}


void FilterData::setData(int index, QVariant data)
{
	vData.insert(index, data);
}

QVariant FilterData::data(int index) const
{
	return vData.value(index);
}


QString FilterData::getText() const
{
	return vData.value(Text, QString("*")).toString();
}

FilterItem* FilterData::item()
{
	if(f.isNull())
		f = new FilterItem(this);
	return f.data();
}

void FilterData::operateFilter(QList<FontItem *>fl)
{
	QList<FontItem*> tmpList = fl;
	QList<FontItem*> negList;
	QList<FontItem*> queList;

	bool negate(vData[Not].toBool());
	bool queue(vData[And].toBool());
	bool append(vData[Or].toBool());

	FMFontDb* fmdb(FMFontDb::DB());

	if(queue)
	{
		queList = fmdb->getFilteredFonts();
	}
	if(negate)
		negList = fmdb->AllFonts();

	if(!append)
		fmdb->clearFilteredFonts();

	if(negate)
	{
		if(queue)
		{
			foreach(FontItem* f, negList)
			{
				if(!tmpList.contains(f) && queList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else if(append)
		{
			foreach(FontItem* f, tmpList)
			{
				if(!fmdb->isFiltered(f) && !tmpList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, negList)
			{
				if(!tmpList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
	}
	else // not negate
	{
		if(queue)
		{
			foreach(FontItem* f, tmpList)
			{
				if(queList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else if(append)
		{
			foreach(FontItem* f, tmpList)
			{
				if(!fmdb->isFiltered(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, tmpList)
			{
				fmdb->insertFilteredFont(f);
			}
		}
	}

	emit Operated();
}
