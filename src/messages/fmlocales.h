#ifndef FMLOCALES_H
#define FMLOCALES_H

#include <QString>
#include <QMap>
#include <QLocale>


QString sysLoc()
{
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
	
	return sys;
	
}

#endif
