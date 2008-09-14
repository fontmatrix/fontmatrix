//
// C++ Interface: fmfontstrings
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMFONTSTRINGS_H
#define FMFONTSTRINGS_H

#include "fmfontdb.h"
#include <QObject>

class FontStrings : public QObject
{
		FontStrings();
		~FontStrings() {}
		static FontStrings* instance;
		static FontStrings* getInstance();

	public:
		enum PanoseKey
		{
			FamilyType = 0,
			SerifStyle,
			Weight,
			Proportion,
			Contrast,
			StrokeVariation,
			ArmStyle,
			Letterform,
			Midline,
			XHeight

		};
		
		static QMap<FMFontDb::InfoItem,QString>& Names();
		static QMap< PanoseKey, QMap<int, QString> >& Panose();
		static QString PanoseKeyName(PanoseKey pk);
		static QString Encoding(FT_Encoding enc);

	private:

		QMap<FMFontDb::InfoItem,QString> m_name;
		void fillNamesMeaning();
		void fillCharsetMap();
		void fillPanoseMap();

		QMap< PanoseKey, QMap<int, QString> > m_panoseMap;
		QMap< PanoseKey, QString > m_panoseKeyName;
		QMap<FT_Encoding, QString> charsetMap;

};

#endif

