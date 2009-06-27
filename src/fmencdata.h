//
// C++ Interface: fmencdata
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMENCDATA_H
#define FMENCDATA_H

#include <QMap>
#include <QString>
#include <QPair>

class FMEncData
{
	static FMEncData * instance;
	FMEncData();
	static FMEncData * that();
	
	QMap<int, QString> langIdMap;
	void fillLangIdMap();
	
	QMap<int, QPair<int,int> > os2URangeMap;
	void fillOs2URAnges();
	
	public:
		static const QMap<int, QString>& LangIdMap();
		static const QMap<int, QPair<int,int> >& Os2URanges();
};


#endif // FMENCDATA_H
