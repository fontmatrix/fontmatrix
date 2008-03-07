//
// C++ Implementation: fmshaper_own
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmshaper_own.h"

#include <QDebug>
#include <QFile>

FMOwnShaper::FMOwnShaper(QString s, QString lang)
{
	loadRules(lang);
	
}

FMOwnShaper::~ FMOwnShaper()
{
}

int FMOwnShaper::loadRules(QString lang)
{
	QFile dictFile(":/shapers/"+lang+".dict");
	if(!dictFile.open(QIODevice::ReadOnly))
	{
		qDebug()<<"Failed to open " << dictFile.fileName();
		return 1;
	}
	QFile matchFile(":/shapers/"+lang+".match");
	if(!matchFile.open(QIODevice::ReadOnly))
	{
		qDebug()<<"Failed to open " << matchFile.fileName();
		return 1;
	}
	
	// lalala
	
	while (!dictFile.atEnd()) {
		QByteArray line = dictFile.readLine();
		if(line.startsWith('%')) // A bit of TeXish in it ;-)
			continue;
		
		QList<QByteArray> elems = line.split ( '|' );
		if(elems.count() > 0)
		{
			bool ok;
			int unicode = elems.takeFirst().mid(0,4).toInt(&ok,16) ;
			if(!ok)
				qDebug()<<"Oops";
			Dictionnary[unicode] = Character(unicode, elems);
		}
		
	}
	while (!matchFile.atEnd()) {
		QByteArray line = matchFile.readLine();
		if(line.startsWith('%'))
			continue;
		
		QList<QByteArray> elems = line.split ( '|' );
		if(elems.count() == 2)
		{
			Matches.append(MatchSequence());
			Replacements.append(ReplaceSequence());
			
			Matches.last().SetMatch(elems[0]);
			Replacements.last().SetReplace(elems[1]);
		}
		
	}
	
}

void FMOwnShaper::fillIn(const QString& s)
{
	for(int i(0); i < s.count(); ++i)
	{
		if(Dictionnary.contains(s[i].unicode()))
		{
			In << Dictionnary[s[i].unicode()];
		}
		else
		{
			In << Character(s[i].unicode());
		}
	}
}

void FMOwnShaper::Op()
{
	/// Here is the beast :)
	
}


void FMOwnShaper::DumpOut()
{
	for(int i(0); i < Out.count(); ++i)
	{
		qDebug()<<"Unicode("<< Out[i].unicode() <<").["<< Out[i].DumpCustom() <<"]";
	}
}





/// Character
Character::Character(int unicode, QList< QByteArray > tokens)
	:QChar(unicode)
{
	for(int i(0); i < tokens.count(); ++i)
		CustomProperties << QString(tokens[i]);
}


Character::Character(int unicode, QStringList tokens)
	:QChar(unicode)
{
	for(int i(0); i < tokens.count(); ++i)
		CustomProperties << tokens[i];
}

QString Character::DumpCustom()
{
	QString ret;
	bool first = true;
	foreach (QString value, CustomProperties)
	{
		if(first)
		{
			ret+= " ; ";
			first =false;
		}
		ret += value;
	}
	return ret;
}



/// Sequences

void MatchSequence::SetMatch(const QByteArray &b)
{
	/*
	 The byte array looks like : 
		"U1111(prop1, prop2, prop3)U2222.(prop4)[U3333-U4444](prop5)"
	And it has to be turned into :
		QList(
			Character(U+1111).CustomProperties["prop1", "prop2", "prop3"],
			Character(U+2222),
			Character(`\0`).CustomProperties["prop4"],
			Character(U+3333).CustomProperties["prop5"],
			Character(U+nnnn).CustomProperties["prop5"],
			Character(U+4444).CustomProperties["prop5"]
		)
	
	The null character will match only on properties. So dot its quite the same as in REGEX.
	*/
	
	QString ref(b);
	for(int idx(0); idx < ref.count(); ++idx)
	{
		QChar current(ref[idx]);
		if(current == 'U') // a code point
		{
			bool ok;
			++idx;
			int unicode = ref.mid(idx,4).toInt(&ok,16) ;
			if(!ok)
				qDebug()<<"Oops";
			idx += 4;
			if(ref[idx] != '(')
			{
				--idx;
				Properties << Character(unicode);
			}
			else // property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
			}
			
			
		}
		else if(current == '.') // a null char (can have properties)
		{
			int unicode = 0 ;
			++idx;
			if(ref[idx] != '(')
			{
				--idx;
				Properties << Character(unicode);
			}
			else // property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
			}
		}
		else if(current == '[') // a code points range 
		{
			/*
			[ U1111 - U2222 ]
			0  2       8    12
			*/
			int beginRange;
			int endRange;
			bool ok;
			beginRange = ref.mid(idx + 2 ,4).toInt(&ok,16) ;
			if(!ok)
				qDebug()<<"Oops";
			endRange = ref.mid(idx + 8 ,4).toInt(&ok,16) ;
			if(!ok)
				qDebug()<<"Oops";
			idx += 13;
			QStringList pList;
			if(ref[idx] != '(')
			{
				--idx;
// 				Properties << Character(unicode);
			}
			else // property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
// 				Properties << Character(unicode, pList);
			}
			for(; beginRange <=  endRange ; ++beginRange)
			{
				Properties << Character(beginRange, pList);
			}
			
		}
		else
		{
			// Error
			qDebug()<<"ERROR";
		}
	}
}

void ReplaceSequence::SetReplace(const QByteArray& b) 
{
	QString ref(b);
	for(int idx(0); idx < ref.count(); ++idx)
	{
		QChar current(ref[idx]);
		if(current == 'U') // a code point
		{
			bool ok;
			++idx;
			int unicode = ref.mid(idx,4).toInt(&ok,16) ;
			if(!ok)
				qDebug()<<"Oops";
			idx += 4;
			if(ref[idx] != '(')
			{
				--idx;
				Properties << Character(unicode);
			}
			else // property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
			}
			
			
		}
		else if(current == '.') // a null char (can have properties)
		{
			int unicode = 0 ;
			++idx;
			if(ref[idx] != '(')
			{
				--idx;
				Properties << Character(unicode);
			}
			else // property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
			}
		}
		else if(current == '[') // a code points range 
		{
			/*
			[ U1111 - U2222 ]
			0  2       8    12
			*/
			int beginRange;
			int endRange;
			bool ok;
			beginRange = ref.mid(idx + 2 ,4).toInt(&ok,16) ;
			if(!ok)
				qDebug()<<"Oops";
			endRange = ref.mid(idx + 8 ,4).toInt(&ok,16) ;
			if(!ok)
				qDebug()<<"Oops";
			idx += 13;
			QStringList pList;
			if(ref[idx] != '(')
			{
				--idx;
// 				Properties << Character(unicode);
			}
			else // property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
// 				Properties << Character(unicode, pList);
			}
			for(; beginRange <=  endRange ; ++beginRange)
			{
				Properties << Character(beginRange, pList);
			}
			
		}
		else
		{
			// Error
			qDebug()<<"ERROR";
		}
	}
}







