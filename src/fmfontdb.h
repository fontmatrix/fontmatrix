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
			Info
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

		/// Here are somehow "actual" public api
		static FMFontDb * DB();
		void TransactionBegin();
		bool TransactionEnd();

		void initRecord ( const QString& id );
		void setValue ( const QString& id, Field field, QVariant value );
		void setValues ( const QString& id, QList<Field> fields, QVariantList values );

		void setInfoMap ( const QString& id, const FontInfoMap& info );

		QVariant getValue ( const QString& id, Field field );
		FontInfoMap getInfoMap ( const QString& id );

		FontItem* Font ( const QString& id );

		QList<FontItem*> AllFonts();
		QStringList AllFontNames();
		FontItem* FirstFont();
		FontItem* NextFont();

		QList<FontItem*> Fonts ( const QVariant& pattern, Field field );
		QList<FontItem*> Fonts ( const QVariant& pattern, InfoItem info, int codeLang = 0 );
		QList<FontItem*> Fonts ( const QString& whereString, Table table );
		int FontCount();
		
		QStringList getTags();
		void addTagToDB(const QString& t);
		void addTag(const QString& id, const QString& t);
		void removeTag(const QString& id, const QString& t);
		void setTags(const QString& id, const QStringList& tl);
		
		bool Remove(const QString& id);

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

		QList<QSqlError> transactionError;
		
		int transactionDeep;
};


#endif
