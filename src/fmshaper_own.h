//
// C++ Interface: fmshaper_own
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMSHAPER_OWN_H
#define FMSHAPER_OWN_H

#include <QSet>
#include <QString>
#include <QStringList>
#include <QMap>

/**
	The shaper, as I understand it, is aimed to prepare
	a string in the "character space" while glyphs
	will be processed in a Harfbuzz buffer.

	06/03/2008 - I do a very naive version, to see.
*/

class Character : public QChar
{
	public: 
		Character(int unicode, QList<QByteArray> tokens);
		Character(int unicode, QStringList tokens);
		Character(int unicode):QChar(unicode),MatchAll(false){}
		Character():QChar(),MatchAll(false){}
		// it should rather be a QFlag... if only I knew how it works ;-)
		QSet<QString> CustomProperties;
		
		// Do we want to match all CustomProperties
		bool MatchAll;
		
		// GroupIndex will be used for replacement
		/* I think it needs further explanations:
		
		U1111(propA).(propB).(propC)|.2(propC,propD)U2222(propA).1(propE)
		
		In this example, we want to reorder a sequence while matching is
		based on properties only. So we need to map matched positions to
		replacement positions as in "grep" group mechanism.
		*/
		bool isMatchedGroup;
		int GroupIndex;
		QString DumpCustom();
		
};


/// All the challenge comes from the fact that we want to match 
/// over an whole properties List.
struct MatchSequence
{
	void SetMatch(const QByteArray&);
	
	QList<Character> Properties;
};

struct ReplaceSequence
{
	void SetReplace(const QByteArray&);
	
	QList<Character> Properties;
};

class FMOwnShaper
{
	public:
		FMOwnShaper(QString s, QString lang);
		~FMOwnShaper();
	private:
		QList<Character> In;
		QList<Character> Out;
		
		QMap<int, Character> Dictionnary;

		// < property, feature >
		QMap<QString, QString> ProperyMap;
		
		QList<MatchSequence> Matches;
		QList<ReplaceSequence> Replacements;
		
		
		int loadRules(QString lang);
		void fillIn(const QString& s);
		
		int Compare(int inIndex, int matchIndex);
		void Replace(int repIndex, QList<Character> chunk);
		void Op();
		
	public:
		
		void DumpOut();
		QList<Character> GetShaped();
};

#endif

