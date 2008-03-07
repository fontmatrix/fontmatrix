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
#ifndef FMSHAPER_OWN
#define FMSHAPER_OWN

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
		Character(int unicode):QChar(unicode){}
		Character():QChar(){}
		// it should rather be a QFlag... if only I knew how it works ;-)
		QSet<QString> CustomProperties;
		
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
		QList<Character> Buffer;
		
		QMap<int, Character> Dictionnary;

		// < property, feature >
		QMap<QString, QString> ProperyMap;
		
		QList<MatchSequence> Matches;
		QList<ReplaceSequence> Replacements;
		
		
		int loadRules(QString lang);
		void fillIn(const QString& s);
	public:
		void Op();
		void DumpOut();
};

#endif

