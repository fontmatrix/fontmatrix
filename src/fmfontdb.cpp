//
// C++ Implementation: fmfontdb
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmfontdb.h"
#include "typotek.h"
#include "fontitem.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlDriver>
#include <QDebug>




FMFontDb * FMFontDb::instance = 0;
FMFontDb::FMFontDb()
		:QSqlDatabase ( typotek::getInstance()->getDatabaseDriver() )
{
	fieldName[FontId]	= "fontident";
	fieldName[Id]		= "digitident";
	fieldName[Tags] 	= "tag";
	fieldName[Lang] 	= "lang";
	fieldName[InfoKey] 	= "infokey";
	fieldName[InfoValue] 	= "infovalue";
	fieldName[Type] 	= "type";
	fieldName[FsType] 	= "fstype";
	fieldName[Family] 	= "family";
	fieldName[Variant] 	= "variant";
	fieldName[Name] 	= "name";
	fieldName[Panose] 	= "panose";
	fieldName[FileSize] 	= "filesize";
	fieldName[Activation] 	= "activation";

	tableName[InternalId]	= "fontmatrix_id";
	tableName[Data] 	= "fontmatrix_data";
	tableName[Tag] 		= "fontmatrix_tags";
	tableName[Info] 	= "fontmatrix_info";

	getIdStringFast = "SELECT %1 FROM %2 WHERE %3='%4'";

	transactionDeep = 0;
}

FMFontDb * FMFontDb::DB()
{
	if ( !instance )
	{
		instance = new FMFontDb;
		instance->initFMDb();
		Q_ASSERT ( instance );
	}
	return instance;
}

void FMFontDb::initRecord ( const QString & id )
{
// 	qDebug()<<"initRecord"<<id;
	int nId ( ++internalCounter );
	QVariantList idlist;
	QVariantList nidlist;
	idlist << id;
	nidlist << nId;
	
	QString qs1 ( QString ( "INSERT INTO %1(%2,%3) VALUES(?,?)" )
	              .arg ( tableName[InternalId] )
	              .arg ( fieldName[FontId] )
	              .arg ( fieldName[Id] )
	            );
	
	QSqlQuery query ( *this );
	query.prepare ( qs1 );
	query.addBindValue( idlist );
	query.addBindValue( nidlist );

	if ( !query.execBatch() )
	{
		transactionError << lastError();
		qDebug()<< "Error initialize record"<< id << lastError().text();
// 		Q_ASSERT(0);
	}
	else // if not in internal id, no need to go further
	{
		QString qs2 ( QString ( "INSERT INTO %1(%2) VALUES(%3)" )
				.arg ( tableName[Data] )
				.arg ( fieldName[Id] )
				.arg ( nId )
			    );
		if ( !query.exec ( qs2 ) )
			transactionError << lastError();
	}

	cacheId[id] = nId;
	reverseCacheId[nId] = id;
}

void FMFontDb::setValue ( const QString & id, Field field, QVariant value )
{
// 	qDebug()<<"setValue"<<id<<fieldName[field]<<value;
	int nId ( getId ( id ) );
	bool res ( false );
// 	transaction();
	if ( field == Tags )
	{
		setTags ( id, value.toStringList() );
	}
	else
	{
		QString qs ( QString ( "UPDATE %1 SET %2='%3' WHERE %4='%5'" )
		             .arg ( tableName[Data] )
		             .arg ( fieldName[field] )
		             .arg ( value.toString() )
		             .arg ( fieldName[Id] )
		             .arg ( nId ) );
		QSqlQuery query ( qs, *this );
		res = query.exec();
	}
	if ( !res )
		transactionError << lastError();
}

void FMFontDb::setValues ( const QString & id, QList< Field > fields, QVariantList values )
{
	int nId ( getId ( id ) );
	bool res ( false );
// 	transaction();
	QVariantList vl;
	for ( int i ( 0 );i<fields.count();++i )
	{
		vl.clear();
		vl << values[i];
		QString qs ( QString ( "UPDATE %1 SET %2=? WHERE %3='%4'" )
		             .arg ( tableName[Data] )
		             .arg ( fieldName[fields[i]] )
// 		             .arg ( values[i].toString() )
		             .arg ( fieldName[Id] )
		             .arg ( nId ) );
		QSqlQuery query (*this );
		query.prepare( qs );
		query.addBindValue( vl );
		
		res = query.execBatch();
		if ( !res )
			break;
	}

	if ( !res )
		transactionError << lastError();
}

void FMFontDb::setInfoMap ( const QString & id, const QMap< int, QMap < int , QString > > & info )
{
// 	qDebug()<<"setInfoMap"<<id;
	// Here is the interesting part :-s)

	// id | lang | key | value
	QTime t;
	t.start();
	int c ( 0 );
	int nId ( getId ( id ) );
	QVariantList idlist;
	QVariantList langlist;
	QVariantList keylist;
	QVariantList valuelist;
	QString qs ( QString ( "INSERT INTO %1 VALUES(?,?,?,?)" )
	             .arg ( tableName[Info] )
	             /*.arg(fieldName[Id])
	             .arg(fieldName[Lang])
	             .arg(fieldName[InfoKey])
	             .arg(fieldName[InfoValue])*/ );
	QSqlQuery query ( *this );
	query.prepare ( qs );
	foreach ( int lang, info.keys() )
	{
		foreach ( int key, info[lang].keys() )
		{
			++c;
			idlist << nId;
			langlist << lang;
			keylist << key;
			valuelist << info[lang][key];
		}
	}
	query.addBindValue ( idlist );
	query.addBindValue ( langlist );
	query.addBindValue ( keylist );
	query.addBindValue ( valuelist );

	if ( !query.execBatch() )
	{
		transactionError << lastError();
	}

// 	qDebug() <<"SETINFO"<<c<<t.elapsed();
}

QVariant FMFontDb::getValue ( const QString & id, Field field, bool useCache )
{
// 	qDebug() <<"getValue"<< fieldName[field] <<id;
	if(useCache)
	{
		if(rValueCache.contains(id))
		{
			if(rValueCache[id].contains(field))
			{
	// 			qDebug() <<"getCachedValue"<< fieldName[field] <<id <<rValueCache[id][field];
				return rValueCache[id][field];
			}
		}
		else
			rValueCache.clear();
	}
	else
		rValueCache.clear();
	
	if ( field == Tags )
	{
		QStringList tl;
		QString qs ( QString ( "SELECT %1 FROM %2 WHERE %3='%4'" )
		             .arg ( fieldName[field] )
		             .arg ( tableName[Tag] )
		             .arg ( fieldName[Id] )
		             .arg ( getId ( id ) ) );
		QSqlQuery query ( qs,*this );
		if ( query.exec() )
		{
			while ( query.next() )
			{
				QString t ( query.value ( 0 ).toString() );
				if ( !tl.contains ( t ) )
					tl << t;
			}
		}
		rValueCache[id][field]  = tl;
		return tl;
	}
	else
	{
		QString qs ( QString ( "SELECT %1 FROM %2 WHERE %3='%4'" )
		             .arg ( fieldName[field] )
		             .arg ( tableName[Data] )
		             .arg ( fieldName[Id] )
		             .arg ( getId ( id ) ) );
		QSqlQuery query ( qs,*this );
		if ( query.exec() )
		{
			if ( query.first() )
			{
				rValueCache[id][field]  = query.value ( 0 );
				return query.value ( 0 );
			}
		}
		return QVariant();
	}
}

QList< FontDBResult > FMFontDb::getValues ( Field field, const QList< FontItem * > & fonts)
{
	QList< FontItem * > lF(fonts);
	if(fonts.isEmpty())
		lF = AllFonts();
// TODO really filter out results according to the font list given in argument
	QList<FontDBResult> ret;
	QString qs ( QString ( "SELECT %1,%2 FROM %3" )
			.arg ( fieldName[Id])
			.arg ( fieldName[field] )
			.arg ( tableName[Data] ));
	QSqlQuery query ( qs,*this );
	
	if ( !query.exec() )
		return ret;
	else
	{
		while ( query.next() )
		{
			int id ( query.value ( 0 ).toInt() );
			if ( id > 0 )
			{
				FontDBResult fr = qMakePair( fontMap.value ( id ), query.value ( 1 ).toString() );
				if(fr.first)
					ret << fr;
			}
		}
		return ret;
	}
}

FontInfoMap FMFontDb::getInfoMap ( const QString & id )
{
// 	qDebug() <<"getInfoMap"<<id;
	FontInfoMap ret;
	if ( id.isEmpty() )
	{
		qDebug() <<"No such font is referenced";
		return ret;
	}
	if ( temporaryFont.contains(id) )
	{
		qDebug() <<"It is a temp Id";
		return temporaryFont[id]->moreInfo();
	}
	QString qs ( QString ( "SELECT * FROM %1 WHERE %2='%3'" )
	             .arg ( tableName[Info] )
	             .arg ( fieldName[Id] )
	             .arg ( getId ( id ) ) );
	QSqlQuery query ( qs,*this );
	if ( query.exec() )
	{
		int lIdx ( query.record().indexOf ( fieldName[Lang] ) );
		int iIdx ( query.record().indexOf ( fieldName[Id] ) );
		int kIdx ( query.record().indexOf ( fieldName[InfoKey] ) );
		int vIdx ( query.record().indexOf ( fieldName[InfoValue] ) );
		int maxIdx ( query.record().count() );
		while ( query.next() )
		{
			ret[query.value ( lIdx ).toInt() ][query.value ( kIdx ).toInt() ] = query.value ( vIdx ).toString();
		}
	}
	else
	{
		qDebug() <<"Error executing query";	
	}
	return ret;

}

QList<FontDBResult> FMFontDb::getInfo ( const QList< FontItem * > & fonts, InfoItem info, int codeLang )
{
	QList<FontDBResult> ret;
	QString where ( QString ( "(%1='%2') AND (%3='%4')" )
	                .arg ( fieldName[InfoKey] )
	                .arg ( info )
	                .arg ( fieldName[Lang] )
	                .arg ( codeLang ) );

	QString qs ( "SELECT %1,%2 FROM %3 WHERE " + where );
	QSqlQuery query ( qs.arg ( fieldName[Id] )
	                  .arg ( fieldName[InfoValue] )
	                  .arg ( tableName[Info] ),
	                  *this );
	if ( !query.exec() )
		return ret;
	else
	{
		while ( query.next() )
		{
			int id ( query.value ( 0 ).toInt() );
			if ( id > 0 )
			{
				FontDBResult fr = qMakePair( fontMap.value ( id ), query.value ( 1 ).toString() );
				if(fr.first)
					ret << fr;
			}
		}
		return ret;
	}
}

void FMFontDb::addTag ( const QString & id, const QString & t )
{
	int nId ( getId ( id ) );
	QString ts ( QString ( "INSERT INTO %1(%2,%3) VALUES('%4','%5')" )
	             .arg ( tableName[Tag] )
	             .arg ( fieldName[Id] )
	             .arg ( fieldName[Tags] )
	             .arg ( nId )
	             .arg ( t ) );
	QSqlQuery query ( ts,*this );
	if ( query.exec() )
		qDebug()<<nId<<"TAGGING SUCCESS";
	else
		qDebug()<<nId<<"TAGGING ERROR";
}

void FMFontDb::addTag(const QStringList & idlist, const QString & t)
{
	QVariantList nidlist;
	QVariantList taglist;
	QString qs ( QString ( "INSERT INTO %1(%2,%3) VALUES(?,?)" )
			.arg ( tableName[Tag] )
			.arg ( fieldName[Id] )
			.arg ( fieldName[Tags] ) );
	QSqlQuery query ( *this );
	query.prepare ( qs );
	foreach ( QString id, idlist)
	{
		nidlist << getId ( id );
		taglist << t;
	}
	query.addBindValue ( nidlist );
	query.addBindValue ( taglist );

	if ( !query.execBatch() )
	{
		transactionError << lastError();
	}

}

void FMFontDb::removeTag ( const QString & id, const QString & t )
{
	int nId ( getId ( id ) );
	QString qs ( QString ( "DELETE FROM %1 WHERE (%2='%3') AND (%4='%5')" )
	             .arg ( tableName[Tag] )
	             .arg ( fieldName[Id] )
	             .arg ( nId )
	             .arg ( fieldName[Tags] )
	             .arg ( t ) );
	QSqlQuery query ( qs,*this );
	query.exec();
}

void FMFontDb::setTags ( const QString & id, const QStringList & tl )
{
	int nId ( getId ( id ) );
	QString qs ( QString ( "DELETE FROM %1 WHERE %2='%3'" )
	             .arg ( tableName[Tag] )
	             .arg ( fieldName[Id] )
	             .arg ( nId ) );
	QSqlQuery query ( qs,*this );
	query.exec();
// 	TransactionBegin();
	foreach ( QString t, tl )
	{
		addTag ( id, t );
	}
// 	TransactionEnd();
}

QStringList FMFontDb::getTags()
{
// 	qDebug() <<"getTags";
	QString qs ( QString ( "SELECT %1 FROM %2" )
	             .arg ( fieldName[Tags] )
	             .arg ( tableName[Tag] ) );
	QSqlQuery query ( qs,*this );
	if ( query.exec() )
	{
		QStringList tl;
		while ( query.next() )
		{
			QString t ( query.value ( 0 ).toString() );
			if ( !tl.contains ( t ) )
				tl << t;
		}
		return tl;
	}
	return QStringList();
}

void FMFontDb::addTagToDB ( const QString & t )
{
// 	qDebug() << "addtag"<< t;
	QString vs ( QString ( "INSERT INTO %1(%2,%3) VALUES('%4','%5')" )
	             .arg ( tableName[Tag] )
	             .arg ( fieldName[Id] )
	             .arg ( fieldName[Tags] )
	             .arg ( 0 )
	             .arg ( t ) );
	QSqlQuery query ( vs,*this );
	query.exec();
}

void FMFontDb::removeTagFromDB(const QString & t)
{
	QString qs ( QString ( "DELETE FROM %1 WHERE %2='%3'" )
			.arg ( tableName[Tag] )
			.arg ( fieldName[Tags] )
			.arg ( t ) );
	QSqlQuery query ( qs,*this );
	query.exec();
}

void FMFontDb::editTag(const QString & tOld, const QString & tNew)
{
	QString qs ( QString ( "UPDATE %1 SET %2='%3' WHERE %4='%5'" )
			.arg ( tableName[Tag] )
			.arg ( fieldName[Tags] )
			.arg ( tNew ) 
			.arg ( fieldName[Tags] )
			.arg ( tOld ) );
	QSqlQuery query ( qs,*this );
	query.exec();
}

void FMFontDb::initFMDb()
{
// 	qDebug() <<"initFMDb";

	setHostName ( typotek::getInstance()->getDatabaseHostname() );
	setDatabaseName ( typotek::getInstance()->getDatabaseDbName() );
	setUserName ( typotek::getInstance()->getDatabaseUser() );
	setPassword ( typotek::getInstance()->getDatabasePassword() );
	if ( !open() )
	{
		qDebug() <<"Connection to"<<hostName() <<"::"<<databaseName() <<"failed miserably!";
		qDebug() <<"====================================================================";
		qDebug() <<lastError();
		qDebug() <<"====================================================================";
		return;
	}
	else
		qDebug() <<"Connection to"<<hostName() <<"::"<<databaseName() <<"SUCCESS!";

	QStringList tl ( tables ( QSql::Tables ) );
	bool allIsAlreadyHere ( true );
	foreach ( QString tn, tableName.values() )
	{
		if ( !tl.contains ( tn ) )
		{
			allIsAlreadyHere = false;
			break;
		}
	}

	if ( !allIsAlreadyHere )
	{
		// We want to create the tables then!
		QString fId	( QString ( "%1 CHAR(255) " ).arg ( fieldName[FontId] ) );
		QString fNumId	( QString ( "%1 INTEGER" ).arg ( fieldName[Id] ) );

		QString fFamily	( QString ( "%1 CHAR(255) " ).arg ( fieldName[Family] ) );
		QString fVariant ( QString ( "%1 CHAR(255) " ).arg ( fieldName[Variant] ) );
		QString fName	( QString ( "%1 CHAR(255) " ).arg ( fieldName[Name] ) );
		QString fType	( QString ( "%1 CHAR(32) " ).arg ( fieldName[Type] ) );
		QString fPanose	( QString ( "%1 CHAR(32) " ).arg ( fieldName[Panose] ) );
		QString fFsType	( QString ( "%1 INTEGER " ).arg ( fieldName[FsType] ) );
		QString fActivation ( QString ( "%1 INTEGER " ).arg ( fieldName[Activation] ) );

		QString fLang	( QString ( "%1 INTEGER " ).arg ( fieldName[Lang] ) );
		QString fInfoKey ( QString ( "%1 INTEGER " ).arg ( fieldName[InfoKey] ) );
		QString fInfoValue ( QString ( "%1 TEXT" ).arg ( fieldName[InfoValue] ) );

		QString fTags	( QString ( "%1 CHAR(255) " ).arg ( fieldName[Tags] ) );


		QString cData ( QString ( "CREATE TABLE %1 (%2,%3,%4,%5,%6,%7,%8,%9)" )
		                .arg ( tableName[Data] )
		                .arg ( fNumId )
		                .arg ( fFamily )
		                .arg ( fVariant )
		                .arg ( fName )
		                .arg ( fType )
		                .arg ( fPanose )
		                .arg ( fFsType )
		                .arg ( fActivation ) );
		QString iData(QString("CREATE INDEX iData ON %1(%2)").arg ( tableName[Data] ).arg(fieldName[Id] ));

		QString cInfo ( QString ( "CREATE TABLE %1 (%2,%3,%4,%5)" )
		                .arg ( tableName[Info] )
		                .arg ( fNumId )
		                .arg ( fLang )
		                .arg ( fInfoKey )
		                .arg ( fInfoValue ) );
		QString iInfo(QString("CREATE INDEX iInfo ON %1(%2)").arg ( tableName[Info] ).arg(fieldName[Id] ));

		QString cTag ( QString ( "CREATE TABLE %1 (%2,%3)" )
		               .arg ( tableName[Tag] )
		               .arg ( fNumId )
		               .arg ( fTags ) );
		QString iTag(QString("CREATE INDEX iTag ON %1(%2)").arg ( tableName[Tag] ).arg(fieldName[Id]));

		QString cId ( QString ( "CREATE TABLE %1 (%2,%3)" )
		              .arg ( tableName[InternalId] )
		              .arg ( fId )
		              .arg ( fNumId )
		            );
		QString iId(QString("CREATE INDEX iId ON %1(%2)").arg ( tableName[InternalId] ).arg(fieldName[FontId] ));

		QSqlQuery query ( *this );
		if ( !query.exec ( cData ) )
			qDebug() <<"ERROR:"<<cData<<"\n---------------------------------\n"<<query.lastError().databaseText();
		if ( !query.exec ( iData ) )
			qDebug() <<"ERROR:"<<iData<<"\n---------------------------------\n"<<query.lastError().databaseText();
		if ( !query.exec ( cInfo ) )
			qDebug() <<"ERROR:"<<cInfo<<"\n---------------------------------\n"<<query.lastError().databaseText();
		if ( !query.exec ( iInfo ) )
			qDebug() <<"ERROR:"<<iInfo<<"\n---------------------------------\n"<<query.lastError().databaseText();
		if ( !query.exec ( cTag ) )
			qDebug() <<"ERROR:"<<cTag<<"\n---------------------------------\n"<<query.lastError().databaseText();
		if ( !query.exec ( iTag ) )
			qDebug() <<"ERROR:"<<iTag<<"\n---------------------------------\n"<<query.lastError().databaseText();
		if ( !query.exec ( cId ) )
			qDebug() <<"ERROR:"<<cId<<"\n---------------------------------\n"<<query.lastError().databaseText();
		if ( !query.exec ( iId ) )
			qDebug() <<"ERROR:"<<iId<<"\n---------------------------------\n"<<query.lastError().databaseText();

		internalCounter = 0;
	}
	else
	{
		internalCounter = 0;
		QSqlQuery query ( *this );
		query.exec ( QString ( "SELECT MAX(%1) FROM %2" )
		             .arg ( fieldName[Id] )
		             .arg ( tableName[InternalId] ) );
		if ( query.exec() )
		{
			if ( query.first() )
			{
				internalCounter = query.value ( 0 ).toInt();
			}
		}
		bool rq ( false );
		/// build ID caches
		QString qs1 ( "SELECT * FROM %1 " );
		rq = query.exec ( qs1.arg ( tableName[InternalId] ) );
		if ( !rq )
		{
			qDebug() <<query.lastQuery();
			qDebug() <<lastError();
			return;
		}
		else
		{
			int idxdigit ( query.record().indexOf ( fieldName[Id] ) );
			int idxfont ( query.record().indexOf ( fieldName[FontId] ) );
			while ( query.next() )
			{
				cacheId[query.value ( idxfont ).toString() ] = query.value ( idxdigit ).toInt();
				reverseCacheId[query.value ( idxdigit ).toInt() ] = query.value ( idxfont ).toString();
			}
		}

		/// build memory font database
		//  which is ment to be a lightweight version of the 
		// file DB and act as a proxy for most of requests.
		QString qs2 ( "SELECT %1,%2,%3,%4,%5 FROM %6" );
		rq = query.exec ( qs2.arg ( fieldName[Id] )
		                  .arg ( fieldName[Family] )
		                  .arg ( fieldName[Variant] )
		                  .arg ( fieldName[Type] )
		                  .arg ( fieldName[Activation] )
		                  .arg ( tableName[Data] ) );
		if ( !rq )
		{
			qDebug() <<query.lastQuery();
			qDebug() <<lastError();
			return ;
		}
		else
		{
			int anId ( 0 );
			QString path;
			int counter ( 0 );
			bool act ( false );
			while ( query.next() )
			{
				anId =  query.value ( 0 ).toInt();
				path = reverseCacheId[anId];
				act = ( query.value ( 4 ).toInt() == 0 ) ? false : true;
				if ( ( path.isEmpty() ) || ( anId == 0 ) )
					continue;
// 				qDebug()<<anId<<++counter<<path;
				if ( fontMap.contains ( anId ) )
					continue;
				else
					fontMap[anId] = new FontItem ( path ,query.value ( 1 ).toString(),query.value ( 2 ).toString() ,query.value ( 3 ).toString(),act );
			}
		}

	}
}

int FMFontDb::getId ( const QString & fontid )
{
	return cacheId.value ( fontid );
}

FontItem * FMFontDb::Font ( const QString & id , bool noTemporary )
{
// 	qDebug() <<"Font"<<id;
	FontItem * fitem ( 0 );
	if ( id.isEmpty() )
		return fitem;
	if ( temporaryFont.contains(id) )
	{
		if(noTemporary)
		{
			delete temporaryFont[id];
			temporaryFont.remove(id);
		}
		else
			return temporaryFont[id];
	}
	int fid ( getId ( id ) );
	if ( fid > 0 )
	{
		if ( fontMap.contains ( fid ) )
		{
			fitem = fontMap.value ( fid );
			fitem->updateItem() ;
		}
		else
			qDebug() <<"ERROR fetching font item"<<id;
	}
	else
	{
// 		qDebug() <<"New font"<< id;
		fitem = new FontItem ( id );
		if ( fitem->isValid() )
		{
			fitem->dumpIntoDB();
			fid = getId ( id );
			if ( fid > 0 )
			{
				fontMap[fid] = fitem;
			}
			else
			{
				delete fitem;
				fitem = 0;
				qDebug() <<"ERROR creating font item"<<id;
			}
		}
		else
		{
			delete fitem;
			fitem = 0;
			qDebug() <<"ERROR creating font item"<<id;
		}
	}
	return fitem;
}


QList< FontItem * > FMFontDb::AllFonts()
{
// 	if(!fontMap.isEmpty())
	return fontMap.values();

}

QStringList FMFontDb::AllFontNames()
{
	return cacheId.keys();
}

void FMFontDb::TransactionBegin()
{
	if ( transactionDeep > 0 )
		++transactionDeep;
	else
	{
		if(transaction())
		{
			transactionError.clear();
			++transactionDeep;
		}
		else
			qDebug()<< "Cannot BEGIN transaction";
// 		qDebug() <<"TransactionBegin";
	}
}

bool FMFontDb::TransactionEnd()
{
// 	qDebug() <<"TransactionEnd"<< (transactionDeep - 1);

	--transactionDeep;
	if ( transactionDeep > 0 )
		return true;
	if ( transactionError.isEmpty() )
	{
		commit();
		return true;
	}
	else
	{
		bool cestGraveDocteur ( false );
		qDebug() <<"ERRORS ==========================================================================";
		foreach ( QSqlError e,transactionError )
		{
			qDebug() <<e;
			if ( e.isValid () )
				cestGraveDocteur = true;
		}
		qDebug() <<"=================================================================================";
		if ( cestGraveDocteur )
		{
			rollback();
			return false;
		}
		else
		{
			commit();
			return true;
		}
	}
}

int FMFontDb::FontCount()
{
	QString qs ( "SELECT COUNT(%1) FROM %2 " );
	QSqlQuery query ( qs.arg ( fieldName[Id] ).arg ( tableName[InternalId] ),*this );
	if ( query.exec() )
	{
		if ( query.first() )
		{
			return query.value ( 0 ).toInt();
		}
	}
	else
		qDebug() <<query.lastError();
	return 0;
}

QList< FontItem * > FMFontDb::Fonts ( const QVariant & pattern, Field field )
{
	if ( ( field == Family )
	        || ( field == Type )
	        || ( field == Variant )
	        || ( field == Name )
	        /*|| ( field == Panose ) */)
		return Fonts ( QString ( "%1='%2'" ).arg ( fieldName[field] ).arg ( pattern.toString() ), Data );
	else if ( field == Tags )
		return Fonts ( QString ( "%1='%2'" ).arg ( fieldName[field] ).arg ( pattern.toString() ), Tag );
	else if ( field == Activation
	          || ( field == FsType ) )
		return Fonts ( QString ( "%1='%2'" ).arg ( fieldName[field] ).arg ( pattern.toInt() ), Data );
	else
		return QList< FontItem * >();
}

QList< FontItem * > FMFontDb::Fonts ( const QVariant & pattern, InfoItem info, int codeLang )
{
	QString qs ( QString ( "(%1='%2') AND (%3='%4') AND (%5 LIKE '\%%6\%')" )
	             .arg ( fieldName[InfoKey] )
	             .arg ( info )
	             .arg ( fieldName[Lang] )
	             .arg ( codeLang )
	             .arg ( fieldName[InfoValue] )
	             .arg ( pattern.toString() ) );
// 	qDebug()<<qs;
	return Fonts ( qs , Info );
}

QList< FontItem * > FMFontDb::Fonts ( const QString & whereString, Table table )
{
	QList< FontItem * > ret;
	QString qs ( "SELECT %1 FROM %2 WHERE " + whereString );
	QSqlQuery query ( qs.arg ( fieldName[Id] )
	                  .arg ( tableName[table] ),
	                  *this );
	if ( !query.exec() )
		return ret;
	else
	{
		QMap<int , FontItem*> reg;
		while ( query.next() )
		{
			int id ( query.value ( 0 ).toInt() );
			if ( id > 0 )
			{
				if( !fontMap.value(id) )
					qDebug()<<"ERROR : DB contains references to id"<<id<<"which is not in fontmap";
				else
					reg[id] = fontMap.value ( id );
			}
		}
		return reg.values();
	}
}

bool FMFontDb::Remove ( const QString & id )
{
	// TODO  implement remove
// 	return true;
// 	qDebug()<<"Remove"<<id;
	int nId ( getId ( id ) );
	
	fontMap.remove(nId);
	reverseCacheId.remove(nId);
	cacheId.remove(id);

	QSqlQuery queryTag ( QString ( "DELETE FROM %1 WHERE %2='%3'" )
	                     .arg ( tableName[Tag] )
	                     .arg ( fieldName[Id] )
	                     .arg ( nId ),
	                     *this );

	QSqlQuery queryInfo ( QString ( "DELETE FROM %1 WHERE %2='%3'" )
	                      .arg ( tableName[Info] )
	                      .arg ( fieldName[Id] )
	                      .arg ( nId ) ,
	                      *this );

	QSqlQuery queryData ( QString ( "DELETE FROM %1 WHERE %2='%3'" )
	                      .arg ( tableName[Data] )
	                      .arg ( fieldName[Id] )
	                      .arg ( nId ) ,
	                      *this );

	QSqlQuery queryInternal ( QString ( "DELETE FROM %1 WHERE %2='%3'" )
	                          .arg ( tableName[InternalId] )
	                          .arg ( fieldName[Id] )
	                          .arg ( nId ) ,
	                          *this );
	
	if(!queryTag.exec())
		return false;
	if(!queryInfo.exec())
		return false;
	if(!queryData.exec())
		return false;
	if(!queryInternal.exec())
		return false;
	
	
	return true;
}

// FontItem * FMFontDb::FirstFont()
// {
// }
// 
// FontItem * FMFontDb::NextFont()
// {
// }

bool FMFontDb::insertTemporaryFont ( const QString & path )
{
	// basic check
// 	qDebug() <<"TFont"<<path;
	if ( path.isEmpty() )
		return false;

	QFileInfo fi ( path );
	QString absPath ( fi.absoluteFilePath() );

	if ( temporaryFont.contains ( path ) )
	{
		return true;
	}

	// check if we have it yet
	int fid ( getId ( absPath ) );
	if ( fid > 0 )
	{
		if ( fontMap.contains ( fid ) )
		{
			return true;
		}
	}

	// Build an item
	FontItem *item ( new FontItem ( absPath ) );
	if ( !item->isValid() )
	{
		delete item;
		return false;
	}
	temporaryFont[absPath] = item;
// 	item->lock();
	return true;

}
















