//
// C++ Interface: fmhyphenator
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMHYPHENATOR_H
#define FMHYPHENATOR_H

#include "hyphen.h"

#include "fmsharestruct.h"

#include <QObject>
#include <QString>
#include <QPair>
#include <QList>

typedef QMap<int , QPair<QString, QString>  > HyphList;
		
class FMHyphenator : public QObject
{
	public:
		FMHyphenator(const QString& dictPath);
		~FMHyphenator();
		
		HyphList hyphenate(const QString& word) const;
		
	private:
	HyphenDict *dict;
};

#endif

