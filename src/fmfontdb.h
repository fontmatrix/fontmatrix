//
// C++ Interface: fmfontdb
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMFONTDB_H
#define FMFONTDB_H

#include "fontitem.h"

#include <QSqlDatabase>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QSqlError>

/**
* An effort to migrate font storage to proper database.
* We expect to avoid the growing memory print when running
* and speedup starting of the program.
*
*
*/

typedef QPair<FontItem*, QString> FontDBResult;

class FMFontDb : public QSqlDatabase
{
		// No surprise, we want it to be a singleton
		static FMFontDb *instance;
		FMFontDb();
		~FMFontDb() {}
		enum Table
		{
			InternalId = 0,
			Data = 10,
			Tag,
			Info,
   			TOs2
		};
	public:
		enum Field
		{
			FontId = 0,
			Id = 1,
			Activation = 2,
			Type = 10,
			Family,
			Variant,
			Name,
			Panose,
			FileSize,
			FsType,
			Tags,
			Lang,
			InfoKey,
			InfoValue,
		};
		enum InfoItem
		{
			Copyright=0,
			FontFamily=1,
			FontSubfamily=2,
			UniqueFontIdentifier=3,
			FullFontName=4,
			VersionString=5,
			PostscriptName=6,
			Trademark=7,
			ManufacturerName=8,
			Designer=9,
			Description=10,
			URLVendor=11,
			URLDesigner=12,
			LicenseDescription=13,
			LicenseInfoURL=14,
// 					 	Reserved=15,
			PreferredFamily=16,
			PreferredSubfamily=17,
			CompatibleMacintosh=18,
			SampleText=19,
			PostScriptCIDName=20,
			AllInfo = 99
		};
		// http://www.microsoft.com/typography/OTSpec/os2.htm
// 		enum OS2
// 		{
// 			Os2_Version = 0,
// 			Os2_xAvgCharWidth,
// 			Os2_usWeightClass,
// 			Os2_usWidthClass,
// 			Os2_fsType,
// 			Os2_ySubscriptXSize,
// 			Os2_ySubscriptYSize,
// 			Os2_ySubscriptXOffset,
// 			Os2_ySubscriptYOffset,
// 			Os2_ySuperscriptXSize,
// 			Os2_ySuperscriptXOffset,
// 			Os2_ySuperscriptYOffset,
// 			Os2_yStrikeoutSize,
// 			Os2_yStrikeoutPosition,
// 			Os2_sFamilyClass,
// 			Os2_panose,
// 			Os2_ulUnicodeRange1,
// 			Os2_ulUnicodeRange2,
// 			Os2_ulUnicodeRange3,
// 			Os2_ulUnicodeRange4,
// 			Os2_achVendID,
// 			Os2_fsSelection,
// 			Os2_usFirstCharIndex,
// 			Os2_usLastCharIndex,
// 			Os2_sTypoAscender,
// 			Os2_sTypoDescender,
// 			Os2_sTypoLineGap,
// 			Os2_usWinAscent,
// 			Os2_usWinDescent,
// 			Os2_ulCodePageRange1,
// 			Os2_ulCodePageRange2,
// 			Os2_sxHeight,
// 			Os2_sCapHeight,
// 			Os2_usDefaultChar,
// 			Os2_usBreakChar,
// 			Os2_usMaxContext
// 		};
		/// Here are somehow "actual" public api
		static FMFontDb * DB();
		void TransactionBegin();
		bool TransactionEnd();

		void initRecord ( const QString& id );
		void setValue ( const QString& id, Field field, QVariant value );
		void setValues ( const QString& id, QList<Field> fields, QVariantList values );

		void setInfoMap ( const QString& id, const FontInfoMap& info );

		QVariant getValue ( const QString& id, Field field, bool useCache = true );
		QList<FontDBResult> getValues ( const QList<FontItem*>& fonts, Field field );
		FontInfoMap getInfoMap ( const QString& id );
		QList<FontDBResult> getInfo ( const QList<FontItem*>& fonts, InfoItem info, int codeLang = 0 );
		

		FontItem* Font ( const QString& id , bool noTemporary = false );

		QList<FontItem*> AllFonts();
		QStringList AllFontNames();
// 		FontItem* FirstFont();
// 		FontItem* NextFont();

		QList<FontItem*> Fonts ( const QVariant& pattern, Field field );
		QList<FontItem*> Fonts ( const QVariant& pattern, InfoItem info, int codeLang = 0 );
		QList<FontItem*> Fonts ( const QString& whereString, Table table );
		int FontCount();

		QStringList getTags();
		void addTagToDB ( const QString& t );
		void removeTagFromDB ( const QString& t );
		void editTag (const QString& tOld, const QString& tNew);
		void addTag ( const QString& id, const QString& t );
		void addTag ( const QStringList& idlist, const QString& t );
		void removeTag ( const QString& id, const QString& t );
		void setTags ( const QString& id, const QStringList& tl );

		bool Remove ( const QString& id );
		bool insertTemporaryFont ( const QString& path );

	private:
		// ensure tables are created
		void initFMDb();

		QMap<Field, QString> fieldName;
		QMap<Table, QString> tableName;
		QMap<QString,int> cacheId;
		QMap<int, QString> reverseCacheId;

		QString getIdStringFast;
		int getId ( const QString& fontid );
		int internalCounter;

		// We maintain a map of instanciated fonts
		QMap<int,FontItem*> fontMap;
		QMap<int,FontItem*>::iterator fontCursor;
		void clearFontMap();
		QMap<QString , FontItem*> temporaryFont;

		QList<QSqlError> transactionError;
		QMap<QString, QMap<Field, QVariant> > rValueCache;

		int transactionDeep;

};


#endif
