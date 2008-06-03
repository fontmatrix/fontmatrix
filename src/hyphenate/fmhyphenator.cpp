//
// C++ Implementation: fmhyphenator
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmhyphenator.h"

#include <QDebug>
#include <QStringList>

FMHyphenator::FMHyphenator()
{
	dict = 0;
}

bool FMHyphenator::loadDict(const QString & dictPath, int leftMin, int rightMin)
{
	/* load the hyphenation dictionary */ 
	if(dict) 
	{
		if(dictPath != currentDictPath)
			hnj_hyphen_free (dict);
		else 
		{
			if(dict)
				return true;
			else 
				return false;
		}
	}

	if (( dict = hnj_hyphen_load( dictPath.toLocal8Bit() ) ) == 0)
	{
		qDebug()<<"Unable to load dict file:"<<dictPath;
		return false;
	}
	else
	{
		dict->lhmin = leftMin;
		dict->rhmin = rightMin;
	}
	return true;
}


FMHyphenator::~FMHyphenator()
{
	if(dict)
		hnj_hyphen_free (dict);
}



/**
 * 
 * @param word 
 * @return 
 */
HyphList FMHyphenator::hyphenate(const QString & word) const 
{
	HyphList ret;
	if(!dict)
		return ret;
	
// 	QMap<int,QChar> upperLog;
// 	for(int i(0);i<word.count();++i)
// 	{
// 		if(word[i].isUpper())
// 			upperLog[i] = word[i];
// 	}
	
	char ** rep = NULL;
	int * pos = NULL;
	int * cut = NULL;
	QByteArray hw( word.toLower().remove('.').toUtf8() );
	QByteArray ht( hw.count() + 5, '0' );
	char *lcword = hw.data();
	char *hyphens = ht.data();
	
	
	if(hnj_hyphen_hyphenate2(dict, lcword, hw.count(), hyphens, 0, &rep, &pos, &cut))
	{
		qDebug()<<"Hyphenate("<<word<<") failed";
		delete hyphens;
		return ret;
	}
	
	QString ref(word/*.toLower().remove('.')*/);
	for(int i(0); i < ref.count(); ++i)
	{
		if(ht[i] & 1)
		{
			QString left(ref.left(i+1));
			QString right(ref.mid(i+1));
// 			qDebug()<<"IH L R"<< left << right;
			if(rep && rep[i])
			{
				QString ref2(left + QString::fromUtf8( (rep[i]) ).remove("=") + right);
// 				QStringList repList( ref2.split("=") );
				int posI(pos ? pos[i] : 0);
				int cutI(cut ? cut[i] : 0);
				
// 				if(repList.count() != 2)
// 				{
// 					qDebug()<<"OOPS - repList =="<<repList.count() ;
// 					continue;
// 				}
				
				left = ref2.mid(0 , left.count() + posI);
				right = ref2.mid(left.count()  + cutI);
				
				qDebug()<<"L R S C P"<< left<<"=" <<right<<(cut?QString::number( cut[i] ):"-")<<(pos?QString::number( pos[i] ):"-");
			}
			ret[i] = QPair<QString, QString>(left, right);
		}
	}
	
	return  ret;
}


