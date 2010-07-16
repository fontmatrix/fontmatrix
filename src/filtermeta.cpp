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

#include "filtermeta.h"
#include "fmfontdb.h"
#include "fontitem.h"

FilterMeta::FilterMeta():
		FilterData()
{
}


QString FilterMeta::type() const
{
	return QString("Meta");
}

void FilterMeta::operate()
{
	QString fs ( vData[Value].toString() );
	int ff(vData[Field].toInt());

	if(ff == FILTER_FIELD_SPECIAL_UNICODE)  //Unicode
	{
		QList<FontItem*> allList = FMFontDb::DB()->AllFonts();
		QList<FontItem*> tl;
		int startC(0xFFFFFFFF);
		int endC(0);
		int patCount(fs.count());
		for(int a(0); a < patCount; ++a)
		{
			unsigned int ca(fs[a].unicode());
			if( ca < startC)
				startC = ca;
			if(ca > endC)
				endC = ca;
		}

		int superSetCount(allList.count());
		for ( int i =0; i < superSetCount; ++i )
		{
			int cc(allList[i]->countCoverage ( startC, endC ) );
			if ( cc >= patCount )
			{
				tl.append ( allList[i]);
			}
		}

		operateFilter(tl);
	}
//	else if(ff == FMFontDb::AllInfo)
//	{
//		FMFontDb::InfoItem k;

//		tmpList.clear();
//		for(int gIdx(0); gIdx < FontStrings::Names().keys().count() ; ++gIdx)
//		{
//			k = FontStrings::Names().keys()[gIdx];
//			if(k !=  FMFontDb::AllInfo)
//			{
//				tmpList +=  FMFontDb::DB()->Fonts(fs,k);
//			}
//		}

//		operateFilter(tmpList, fs);

//	}
	else
	{
		operateFilter(FMFontDb::DB()->Fonts(fs, FMFontDb::InfoItem(ff)));
	}
}
