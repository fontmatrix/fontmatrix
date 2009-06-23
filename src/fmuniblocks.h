//
// C++ Interface: fmuniblocks
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMUNIBLOCKS_H
#define FMUNIBLOCKS_H

#include <QMap>
#include <QPair>
#include <QString>

class FMUniBlocks
{
	static FMUniBlocks * instance;
	static FMUniBlocks * that();
	
	FMUniBlocks();
	void loadBlocks();
	void recordLine(const QString& line);
	typedef QPair<int,int> bKey;
	QMap<bKey, QString> p;
	bKey c; // current
	bKey f; // first
	bKey l; // last
	
	public:
		static QString firstBlock(int& start, int& end);
		static QString lastBlock(int& start, int& end);
		static QString nextBlock(int& start, int& end);
		static QString currentBlock(int& start, int& end);
		
		static int start(const int& codepoint);
		static int end(const int& codepoint);
		
		static bKey interval(const QString& blockName);
		static QStringList blocks();
		
};

#endif // FMUNIBLOCKS_H
