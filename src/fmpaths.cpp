//
// C++ Implementation: fmpaths
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fmpaths.h"

#include <QApplication>

FMPaths *FMPaths::instance = 0;
FMPaths * FMPaths::getThis()
{
	if(!instance)
		instance = new FMPaths;
	return instance;
}

QString FMPaths::sysLoc()
{
	if(getThis()->FMPathsDB.contains("sysLoc"))
		return getThis()->FMPathsDB["sysLoc"];
	QMap<QLocale::Language, QString> la;
	la[QLocale::Danish] = "da";
	la[QLocale::German] = "de";
	la[QLocale::English] = "en";
	la[QLocale::Finnish] = "fi";
	la[QLocale::French] = "fr";
	la[QLocale::Dutch] = "nl";
	la[QLocale::Norwegian] = "no";
	la[QLocale::Russian] = "ru";
	la[QLocale::Serbian] = "sr";
	la[QLocale::Swedish] = "sv";
	la[QLocale::Ukrainian] = "uk";
	la[QLocale::Chinese] = "zh";
	
	QMap<QLocale::Country, QString> co;
	co[QLocale::China] = "CN";
	co[QLocale::Taiwan] = "TW";
	
	QString sys (la.value(QLocale::system ().language() )) ;
	if(co.contains( QLocale::system ().country()))
	{
		sys += "_";
		sys += co.value( QLocale::system ().country());
	}
	getThis()->FMPathsDB["sysLoc"] = sys;
	return sys;
	
}

QString FMPaths::TranslationsDir()
{
	if(getThis()->FMPathsDB.contains("TranslationsDir"))
		return getThis()->FMPathsDB["TranslationsDir"];
#ifdef PLATFORM_APPLE
	QString QMDirPath = QApplication::applicationDirPath();
	QString dirsep(QDir::separator());
	QMDirPath +=  dirsep + ".." + dirsep + "Resources" + dirsep + "locale" + dirsep;	
#elif _WIN32
	QString QMDirPath = QApplication::applicationDirPath();
	QString dirsep(QDir::separator());
	QMDirPath +=  dirsep + "share" + dirsep + "qm" + dirsep;
#else
	QString QMDirPath = PREFIX;
	QString dirsep(QDir::separator());
	QMDirPath +=  dirsep + "share" + dirsep + "fontmatrix" + dirsep + "qm" + dirsep;
#endif
	getThis()->FMPathsDB["TranslationsDir"] = QMDirPath;
	return QMDirPath;
}

QString FMPaths::TranslationFile()
{
	if(getThis()->FMPathsDB.contains("TranslationFile"))
		return getThis()->FMPathsDB["TranslationFile"];
	
	QString QMFilePathComplete("fontmatrix-" + sysLoc() );
	getThis()->FMPathsDB["TranslationFile"] = QMFilePathComplete;
	return QMFilePathComplete;
}

QString FMPaths::HelpFilePath()
{
	if(getThis()->FMPathsDB.contains("HelpFile"))
		return getThis()->FMPathsDB["HelpFile"];
	QString hf;
#ifdef PLATFORM_APPLE
	hf = ResourcesDir() + "help_" + sysLoc() + ".html";
#elif _WIN32
	hf = ResourcesDir() + "help_" + sysLoc() + ".html";
#else
	hf = ResourcesDir() + "help/help_" + sysLoc() + ".html";
#endif
	getThis()->FMPathsDB["HelpFile"] = hf;
	return getThis()->FMPathsDB["HelpFile"];
}

QString FMPaths::ResourcesDir()
{
	if(getThis()->FMPathsDB.contains("ResourcesDir"))
		return getThis()->FMPathsDB["ResourcesDir"];
#ifdef PLATFORM_APPLE
	QString QMDirPath = QApplication::applicationDirPath();
	QString dirsep(QDir::separator());
	QMDirPath +=  dirsep + ".." + dirsep + "Resources" + dirsep ;	
#elif _WIN32
	QString QMDirPath = QApplication::applicationDirPath();
	QString dirsep(QDir::separator());
	QMDirPath +=  dirsep + "share" + dirsep + "resources" + dirsep;
#else
	QString QMDirPath = PREFIX;
	QString dirsep(QDir::separator());
	QMDirPath +=  dirsep + "share" + dirsep + "fontmatrix" + dirsep + "resources" + dirsep;
#endif
	getThis()->FMPathsDB["ResourcesDir"] = QMDirPath;
	return getThis()->FMPathsDB["ResourcesDir"];
}


