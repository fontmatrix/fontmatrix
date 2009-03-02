/* fmpath.h */
/* Get all useful paths in one place with the hope that */
/* finally we will not need resource.qrc anymore */
#ifndef FMPATHS_H
#define FMPATHS_H

#include <QString>
#include <QMap>
#include <QLocale>
#include <QDir>


class FMPaths
{
		QMap<QString,QString> FMPathsDB;
		FMPaths() {};
		static FMPaths *instance;
		static FMPaths *getThis();
	public:

		static QString sysLoc();

		static QString TranslationsDir();

		static QString TranslationFile();

		static QString ResourcesDir();
		
		static QString HelpDir();
		
		static QString ScriptsDir();
};
#endif
