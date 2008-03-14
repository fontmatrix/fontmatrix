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
#include <QSettings>
#include <QDir>

FMOwnShaper::FMOwnShaper(QString s, QString lang)
{
	// We need to load at least a default rule 
	Character noop;
	noop.GroupIndex = 1;
	Matches << MatchSequence();
	Matches.last().Properties << noop;
	Matches.last().Properties.last().isMatchedGroup = true;
	Replacements << ReplaceSequence();
	Replacements.last().Properties << noop;
	Replacements.last().Properties.last().GroupIndex = 1;
	
	loadRules(lang);
	fillIn(s);
	
}

FMOwnShaper::~ FMOwnShaper()
{
}

int FMOwnShaper::loadRules(QString lang)
{
	QSettings stg;
	QString actualSDir;
	QString ShaperDir(stg.value("ShaperDataDir").toString());
	if(ShaperDir.isEmpty())
	{
		actualSDir = ":/shapers/";
	}
	else
	{
		actualSDir = ShaperDir + QDir::separator() ;
	}
	QFile dictFile(actualSDir +lang+ ".dict");
	if(!dictFile.open(QIODevice::ReadOnly))
	{
		qDebug()<<"Failed to open " << dictFile.fileName();
		return 1;
	}
	QFile matchFile(actualSDir +lang+".match");
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
		QString line ( CleanRule(matchFile.readLine()) );
		
		if(line.startsWith('%'))
			continue;
		
		QList<QString> elems = line.split ( '|' );
		if(elems.count() == 2)
		{
			Matches.append(MatchSequence());
			Replacements.append(ReplaceSequence());
			
			Matches.last().SetMatch(elems[0].trimmed());
			Replacements.last().SetReplace(elems[1].trimmed());
		}
		
	}
	
}

void FMOwnShaper::fillIn(const QString& s)
{
	QStringList debug;
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
		debug << "["+QString::number(s[i].unicode(),16)+"]";
	}
	qDebug()<< debug.join(" ");
	
}

void FMOwnShaper::Op()
{
	/// Here is the beast :)
	/*
	It’s all about processing replacements
	
	To make it easy to read, we’ll first put matched chunks into a map
	then process each chunk in another loop.
	*/
	
	// <index of the match sequence, matched list>
	QList< QPair< int, QList< Character > > > chunks;
	int idx(0);
	bool matched;
	while( idx < In.count() )
	{
		matched = false;
		// We begin at 1 since index 0 is NOOP
		for(int nm(1); nm < Matches.count(); ++nm)
		{
			int rc = Compare( idx , nm);
			if(rc > 0)
			{
				QList<Character> cl;
				for(int nc(0); nc < rc; ++nc)
					cl << In[idx + nc];
				chunks.append( QPair< int, QList< Character > >(nm , cl) );
				idx += rc;
				matched = true;
				break;
			}
		}
		if(!matched)
		{
			In[idx].isMatchedGroup = true;
			QList< Character > ul;
			ul << In[idx];
			chunks.append( QPair< int, QList< Character > >(0 ,ul ) );
			++idx;
		}
		
	}
	
	// Now we apply replacements as defined in the rules file
	QList< QPair< int, QList< Character > > >::const_iterator chunkIt = chunks.begin();
	while(chunkIt != chunks.end())
	{
		Replace(chunkIt->first, chunkIt->second);
		++chunkIt;
	}
	
	
}

/// Return 0 if not matched and number of consumed positions if it matched 
int FMOwnShaper::Compare(int inIndex, int matchIndex)
{
	int matchLen = Matches.at(matchIndex).Properties.count();
	// We need to keep track of matched chars over "."
	QList<int> matchedGroup;
	if(matchLen > (In.count() - inIndex))
		return 0;
	for(int i(0); i < matchLen; ++i)
	{
		Character car = In[inIndex + i];
		Character mat = Matches.at(matchIndex).Properties[i];
		if(mat.isNull())// We’ll just compare properties
		{
			if(mat.MatchAll)
			{
				foreach(QString prop, mat.CustomProperties)
				{
					if(!prop.isEmpty())
					{
					if(!car.CustomProperties.contains(prop))
						return 0;
					}
				}
				foreach(QString prop, car.CustomProperties)
				{
					if(!prop.isEmpty())
					{
					if(!mat.CustomProperties.contains(prop))
						return 0;
					}
				}
			}
			else
			{
				foreach(QString prop, mat.CustomProperties)
				{
					if(!prop.isEmpty())
					{
						if(!car.CustomProperties.contains(prop))
							return 0;
					}
				}
			}
			matchedGroup << inIndex + i;
			
		}
		else 
		{
			if(mat.unicode() != car.unicode())
				return 0;
			else
			{
				if(mat.MatchAll)
				{
					foreach(QString prop, mat.CustomProperties)
					{
						if(!car.CustomProperties.contains(prop))
							return 0;
					}
					foreach(QString prop, car.CustomProperties)
					{
						if(!mat.CustomProperties.contains(prop))
							return 0;
					}
				}
				else
				{
					foreach(QString prop, mat.CustomProperties)
					{
						if(!car.CustomProperties.contains(prop))
							return 0;
					}
				}
			}
			
		}
	}
	// It match
	foreach(int idx, matchedGroup)
	{
		In[idx].isMatchedGroup = true;
// 		qDebug()<<"MATCHED" <<In[idx].unicode() << In[idx].DumpCustom();
	}
	return matchLen;
}

/// Apply replacement rule and append the result to Out
void FMOwnShaper::Replace(int repIndex, QList< Character > chunk)
{
	QList<Character> buffer;
	QMap<int, Character> matchedPos;
	//load matchedPos first
	int mIndex(0);
	foreach(Character car, chunk)
	{
		if(car.isMatchedGroup)
		{
			matchedPos[++mIndex] = car;
			qDebug()<< "Matched "<<  QString::number(car.unicode(), 16 ) << " at pos "<< mIndex ;
		}
	}
	// Let replace :)
	int rIndex(0);
	foreach(Character rep, Replacements[repIndex].Properties)
	{
		if(rep.isNull())
		{
			Character tc(matchedPos[rep.GroupIndex].unicode(), rep.CustomProperties.toList());
			buffer << tc;
		}
		else
		{
			buffer << rep;
		}
	}
	// Push in Out
	foreach(Character b, buffer)
	{
		Out << b;
	}
}



void FMOwnShaper::DumpOut()
{
	qDebug()<<"FMOwnShaper::DumpOut()";
	for(int i(0); i < Out.count(); ++i)
	{
		qDebug()<<"Unicode("<< QString::number(Out[i].unicode(), 16 ) <<").["<< Out[i].DumpCustom() <<"]";
	}
}





/// Character
Character::Character(int unicode, QList< QByteArray > tokens)
	:QChar(unicode),MatchAll(false),isMatchedGroup(false)
{
	for(int i(0); i < tokens.count(); ++i)
		CustomProperties << QString(tokens[i].trimmed());
}


Character::Character(int unicode, QStringList tokens)
	:QChar(unicode),MatchAll(false),isMatchedGroup(false)
{
	for(int i(0); i < tokens.count(); ++i)
		CustomProperties << tokens[i].trimmed();
}

QString Character::DumpCustom()
{
	QString ret;
	bool first = true;
	foreach (QString value, CustomProperties)
	{
		if(first)
		{
			first = false;
			ret += value;
			continue;
		}
		
		ret+= " ; ";
		ret += value;
	}
	return ret;
}



/// Sequences

void MatchSequence::SetMatch(const QString &b)
{
	/*
	 The byte array looks like : 
		"U1111(prop1, prop2, prop3)U2222.(prop4)"
	And it has to be turned into :
		QList(
			Character(U+1111).CustomProperties["prop1", "prop2", "prop3"],
			Character(U+2222),
			Character(`\0`).CustomProperties["prop4"]
		)
	
	The null character will match only on properties. So dot its quite the same as in REGEX.
	*/
	
	QString ref(b);
// 	qDebug()<<"SetMatch("+ref+")";
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
			if(ref[idx] == '(')// property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars-1).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
			}
			else if(ref[idx] == '[')// property list with exact match
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ']')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars-1).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
				Properties.last().MatchAll = true;
			}
			else //if(ref[idx] != '(' || ref[idx] != '[')
			{
				--idx;
				Properties << Character(unicode);
			}
			
			
		}
		else if(current == '.') // a null char (can have properties)
		{
			int unicode = 0 ;
			++idx;
			
			if(ref[idx] == '(')// property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars-1).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
			}
			else if(ref[idx] == '[')// property list with exact match
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ']')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars-1).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties << Character(unicode, pList);
				Properties.last().MatchAll = true;
			}
			else //if(ref[idx] != '('|| ref[idx] != '[')
			{
				--idx;
				Properties << Character(unicode);
			}
		}
		else
		{
			// Error
			qDebug()<<"ERROR match: current = "<< current;
		}
	}
}

void ReplaceSequence::SetReplace(const QString& b) 
{
	QString ref(b);
// 	qDebug()<<"SetReplace("+ref+")";
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
				QStringList pl(ref.mid(idx+1, countChars-1).split(";", QString::SkipEmptyParts));
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
			Properties << Character(unicode);
			++idx;
			
			bool ok;
			int group = ref.mid(idx,1).toInt(&ok,10) ;
			Properties.last().GroupIndex = group;
			++idx;
			
			if(ref[idx] != '(')
			{
				--idx;
				
			}
			else // property list
			{
				QStringList pList;
				int countChars(0);
				while(ref[idx + countChars] != ')')
				{
					++countChars;
				}
				QStringList pl(ref.mid(idx+1, countChars-1).split(";", QString::SkipEmptyParts));
				foreach(QString prop, pl)
				{
					pList << prop.trimmed();
				}
				idx += countChars;
				Properties.last().CustomProperties = pList.toSet();
			}
		}
		else
		{
			// Error
			qDebug()<<"ERROR replace: current = "<< current;
		}
	}
}

QList< Character > FMOwnShaper::GetShaped()
{
	Op();
	return Out;
}

QString FMOwnShaper::CleanRule(QString rule)
{
	QString ret;
	int len( rule.count() );
	for(int i(0); i < len; ++i)
	{
		if(!rule[i].isSpace())
			ret += rule[i];
	}
	return ret;
}







