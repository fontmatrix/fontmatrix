//
// C++ Implementation: fmuniblocks
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmuniblocks.h"
#include "fmpaths.h"

#include <QDir>
#include <QFile>
#include <QObject>

#include <QDebug>

FMUniBlocks * FMUniBlocks::instance = 0;
FMUniBlocks::FMUniBlocks()
{
	loadBlocks();
	Q_ASSERT(!p.isEmpty());
	
	f = c = p.keys().first();
	l = p.keys().last();
	
}

void FMUniBlocks::loadBlocks()
{
	// First determine the file we want to load
// 	QString sep(QDir::separator());
// 	QString Unicode("Unicode");
// 	QString Blocks("Blocks-%1.txt");
// 	QString locBlock(FMPaths::ResourcesDir()+Unicode+sep+Blocks.arg(FMPaths::sysLoc()));
// 	
// 	if(QFile::exists(locBlock))
// 	{
// 		QFile bFile(locBlock);
// 		if(bFile.open(QIODevice::ReadOnly))
// 		{
// 			while(!bFile.atEnd())
// 			{
// 				QByteArray ba(bFile.readLine(456));
// 				recordLine(QString::fromUtf8(ba.data(),ba.length()));
// 			}
// 		}
// 	}
// 	else // lets try the vanilla file
// 	{
// 		Blocks = "Blocks.txt";
// 		locBlock = FMPaths::ResourcesDir()+Unicode+sep+Blocks;
// 		if(QFile::exists(locBlock))
// 		{
// 			QFile bFile(locBlock);
// 			if(bFile.open(QIODevice::ReadOnly))
// 			{
// 				while(!bFile.atEnd())
// 				{
// 					QByteArray ba(bFile.readLine(456));
// 					recordLine(QString::fromLocal8Bit(ba.data(),ba.length()));
// 				}
// 			}
// 		}
// 		else
// 			qDebug()<<"No blocks file in sight:"<<locBlock;
// 	}
// 	
	
#include "langs/unicode/uniblocks.cxx"
}


void FMUniBlocks::recordLine(const QString& line)
{
	if(line.startsWith("#"))
		return;
	if(line.trimmed().isEmpty())
		return;
	
	QString rs(line);
	rs.replace("..",";");
	QStringList rl(rs.split(";",QString::SkipEmptyParts));
	if(rl.count() != 3)
	{
		qDebug()<<"ERROR: spliting a block record in"<<rl.count()<<"lines";
		return;
	}
	bool ok;
	int ss(rl[0].toInt(&ok, 16));
	int ll(rl[1].toInt(&ok, 16));
	p[qMakePair<int,int>(ss,ll)] = rl[2].trimmed();
}


FMUniBlocks * FMUniBlocks::that()
{
	if(!instance)
	{
		instance = new FMUniBlocks;
		Q_ASSERT(instance);
	}
	return instance;
}

QString FMUniBlocks::firstBlock(int & start, int & end)
{
	that()->c = that()->f;
	start = that()->c.first;
	end = that()->c.second;
	return that()->p.value(that()->c);
}

QString FMUniBlocks::lastBlock(int & start, int & end)
{
	that()->c = that()->l;
	start = that()->c.first;
	end = that()->c.second;
	return that()->p.value(that()->c);
}

QString FMUniBlocks::nextBlock(int & start, int & end)
{
	if(that()->c == that()->l)
	{
		start = end =0;
		return QString();
	}
	bool current(false);
	
	foreach( bKey k, that()->p.keys() )
	{
		if(current)
		{
			that()->c = k;
			start = that()->c.first;
			end = that()->c.second;
			return that()->p.value(that()->c);
		}
		if(k == that()->c)
			current = true;
	}
	return QString();
}

QString FMUniBlocks::currentBlock(int & start, int & end)
{
	start = that()->c.first;
	end = that()->c.second;
	return that()->p.value(that()->c);
}


int FMUniBlocks::start(const int & codepoint)
{
	foreach(  bKey k, (that()->p.keys()))
	{
		if((codepoint >= k.first) 
				  && (codepoint <= k.second))
			return k.first;
	}
	return -1;
}

int FMUniBlocks::end(const int & codepoint)
{
	foreach(  bKey k , (that()->p.keys()))
	{
		if((codepoint >= k.first) 
				  && (codepoint <= k.second))
			return k.second;
	}
	return -1;
}

FMUniBlocks::bKey FMUniBlocks::interval(const QString & blockName)
{
	foreach(bKey k, that()->p.keys())
	{
		if(that()->p.value(k) == blockName)
			return k;
	}
	qDebug()<<"WARNING: cannot find block name"<<blockName;
	return bKey(0,0);
}

QStringList FMUniBlocks::blocks()
{
	return QStringList(that()->p.values());
}

QString FMUniBlocks::block(bKey key)
{
	QString ret;
	if(that()->p.contains(key))
		ret = that()->p[key];
	return ret;
}







