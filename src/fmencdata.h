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

class FMEncData
{
	static FMEncData * instance;
	FMEncData();
	static FMEncData * that();
	
	QMap<int, QString> langIdMap;
	void fillLangIdMap();
	
	public:
		static const QMap<int, QString>& LangIdMap();
};


#endif // FMENCDATA_H
