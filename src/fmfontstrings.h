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

#include "fontitem.h"
#include "fmfontdb.h"
#include <QObject>

class FontStrings : public QObject
{
	Q_OBJECT
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
			XHeight,
   			InvalidPK = 9999999

		};
		static PanoseKey firstPanoseKey();
		static PanoseKey nextPanoseKey(PanoseKey pk);
		
		static const QMap<FMFontDb::InfoItem,QString>& Names();
		static const QMap< PanoseKey, QMap<int, QString> >& Panose();
		static const QString PanoseKeyName(PanoseKey pk);
		static const QString PanoseKeyInfo(PanoseKey pk);
		static const QString Encoding(FT_Encoding enc);
		static const QMap<QString,QString>& Tables(); 
		static QString FsType(int fstype_part, bool shortString = true);

	private:

		QMap<FMFontDb::InfoItem,QString> m_name;
		void fillNamesMeaning();
		void fillCharsetMap();
		void fillPanoseMap();
		void panoseFromFile(const QString& path);
		void fillTTTableList();
		void fillFSftypeMap();
		
		QMap< PanoseKey, QMap<int, QString> > m_panoseMap;
		QMap< PanoseKey, QString > m_panoseKeyName;
		QMap< PanoseKey, QString > m_panoseKeyInfo;
		QMap<FT_Encoding, QString> charsetMap;
		QMap<QString,QString> tttableList;// <TT name, description>
		QMap< int, QString > m_FsType;
};

#endif

