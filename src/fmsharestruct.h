//
// C++ Interface: fmsharestruct
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMSHARESTRUCT_H
#define FMSHARESTRUCT_H

#include <QList>
// #include <QMap>
#include <QPair>
#include <QDebug>

#include <QString>
#include <QStringList>

struct OTFSet
{
	QString script;
	QString lang;
	QStringList gpos_features;
	QStringList gsub_features;
	OTFSet() {};
	OTFSet ( const OTFSet& os )
	: script ( os.script ),
		   lang ( os.lang ),
			  gpos_features ( os.gpos_features ),
					  gsub_features ( os.gsub_features ) {};
					  QString dump() {return script + "|" +lang+ "|"+ gpos_features.join ( "|" ) + gsub_features.join ( "|" );}
					  bool isEmpty() { return script.isEmpty() && lang.isEmpty() && gpos_features.isEmpty() &&  gsub_features.isEmpty() ;};
};


struct RenderedGlyph
{
	int glyph;
	int log;
	double xadvance;
	double yadvance;
	double xoffset;
	double yoffset;
	unsigned short lChar;
	/**
	As it’s quite usual with hyphenation, it’s not very clear!
	hyphenKey indicates if it’s breakable (>0) and is a key to retrieve
	left and right rendered string of the broken word this glyph belongs.
	e.g.: rendered word == "[p][u][p][p][y]" and this glyph is index 2
	We have hyphenKey == 1 and hyphenValue.value(1).first == [p][u][p][-] and hyphenValue.value(1).second == [p][y]
	If it’s still not clear, e-mail me - pm
	Agh, it was indeed over complicated! - pm
	*/
// 	unsigned short hyphenKey;
	bool isBreak;
	QPair<QList<RenderedGlyph>, QList<RenderedGlyph> > hyphen;
	
	RenderedGlyph():glyph(0),log(0),xadvance(0),yadvance(0),xoffset(0),yoffset(0),lChar(0),isBreak(false){}
	RenderedGlyph(int g,int l,double xa,double ya,double xo,double yo,unsigned short c,bool b)
	:glyph(g),log(l),xadvance(xa),yadvance(ya),xoffset(xo),yoffset(yo),lChar(c),isBreak(b){}

	bool operator==( const RenderedGlyph& other ) const{
		if(glyph != other.glyph)
			return false;
		else if(lChar != other.lChar)
			return false;
		else if(xadvance != other.xadvance)
			return false;
		else if(yadvance != other.yadvance)
			return false;
		else if(xoffset != other.xoffset)
			return false;
		else if(yoffset != other.yoffset)
			return false;
		else if(isBreak != other.isBreak)
			return false;
		return true;
	}
	
	void dump() const
	{
		QString ds("glyph %1; log %2; xadv %3; yadv %4; xof %5; yof %6; char %7; hyph %8");
		qDebug() << ds.arg(glyph)
				.arg(log)
				.arg(xadvance)
				.arg(yadvance)
				.arg(xoffset)
				.arg(yoffset)
				.arg(QChar(lChar))
				.arg(hyphen.first.count());
	}
};

typedef QList<RenderedGlyph> GlyphList;
typedef QPair<QList<RenderedGlyph>, QList<RenderedGlyph> > RenderedHyph;


typedef QMap<int,QMap<int, QString> > FontInfoMap;

#endif
