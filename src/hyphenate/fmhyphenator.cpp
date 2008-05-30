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

FMHyphenator::FMHyphenator(const QString& dictPath)
{
	/* load the hyphenation dictionary */  
	
	if (( dict = hnj_hyphen_load( dictPath.toLocal8Bit() ) ) == 0)
	{
		qDebug()<<"Unable to load dict file:"<<dictPath;
	}
	
}

FMHyphenator::~FMHyphenator()
{
	
}



HyphList FMHyphenator::hyphenate(const QString & word) const 
{
	HyphList ret;
	if(!dict)
		return ret;
	
	
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
	
	QString ref(word.toLower().remove('.'));
	for(int i(0); i < ref.count(); ++i)
	{
		if(ht[i] & 1)
		{
			QString left(ref.left(i));
			QString right(ref.right(i));
			if(rep && rep[i])
			{
				QStringList repList( QString::fromUtf8( (rep[i]) ).split("=") );
				if(repList.count() != 2)
				{
					qDebug()<<"OOPS - repList =="<<repList.count() ;
					continue;
				}
				if(cut)
				{
					left.chop( cut[i] );

				}
				if(pos)
				{
					right.remove(0, pos[i]);
				}
				left.append( repList[0] );
				right.insert(0, repList[1]);
				
			}
			ret[i] = QPair<QString, QString>(left, right);
		}
	}
	
	return  ret;
}


