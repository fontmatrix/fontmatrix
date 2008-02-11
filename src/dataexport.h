//
// C++ Interface: dataexport
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DATAEXPORT_H
#define DATAEXPORT_H

#include <QString>
#include <QDir>

class FontItem;

/**
	@author Pierre Marchand <pierremarc@oep-h.com>
	
	This class is supposed to export a set of fonts in a
	directory and build a usefull index of the exported font files.
*/
class DataExport
{
	public:
		DataExport(const QString &dirPath, const QString &filterTag);
		~DataExport();
		
		int doExport();

	private:
		//data
		QDir exDir;
		QString filter;
		QList<FontItem*> fonts;
		//methods
		int copyFiles();
		int buildIndex();

};

#endif
