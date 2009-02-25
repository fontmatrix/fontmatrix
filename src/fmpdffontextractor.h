//
// C++ Interface: fmpdffontextractor
//
// Description: A dead simple font extractor from PDF docs
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMPDFFONTEXTRACTOR
#define FMPDFFONTEXTRACTOR

#include <QMap>
#include <QStringList>
#include <QIODevice>

#include <podofo/podofo.h>

class FMPDFFontExtractor
{
	public:
		FMPDFFontExtractor(const QString& pdfFile);
		~FMPDFFontExtractor();
		
		QStringList list();
		QString fontType(const QString& name);
		bool write(const QString& name, QIODevice * openedDevice);
	
	private:
		bool cachedList;
		PoDoFo::PdfMemDocument * document;
		QMap<QString, PoDoFo::PdfObject*> mfont;
		QMap<QString, QString> mType;
		
};

#endif // FMPDFFONTEXTRACTOR

