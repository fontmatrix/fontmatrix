//
// C++ Interface: fmaltcontext
//
// Description: Maintains state of user defined alternate glyphs in specific context
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMALTCONTEXT_H
#define FMALTCONTEXT_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>

class FMAltContext
{
	int runPar;
	int runWord;
	int runChunk;
	/**
	  We store lists of alternates at the glyph level in the intermediate form of the text
	  Text
	  ----> Paragraph
		----> Word
		      ----> Chunk (as returned by hyphenator, and actually sent to OT processor)
			    ----> Glyph
				  ----> List<Alternates>
	*/
	// alternate lists
	QMap<int , QMap< int , QMap<int , QMap< int, QList<int> > > > > m_alt;
	// base glyph
	QMap<int , QMap< int , QMap<int , QMap< int, int> > > > m_control;
	// selected glyph (indexOf, 0 by default)
	QMap<int , QMap< int , QMap<int , QMap< int, int> > > > m_select;

	// actual words
	QMap<int , QMap< int , QString> > m_words;
	// actual chunks
	QMap<int , QMap< int , QMap<int , QString> > > m_chunks;

	struct run
	{
		int par;
		int word;
		int chunk;
	};

	QList<run> runStore;
	
public:
	QString fontID;
	QString textID;
	FMAltContext():runPar(0),runWord(0),runChunk(0){}

//	void reset()
//	{
//		m_alt.clear();
//		m_control.clear();
//		m_select.clear();
//
//		runChunk = 0;
//		runPar = 0;
//		runWord = 0;
//	}

	void saveRun(){run r;r.chunk = runChunk;r.par = runPar; r.word = runWord; runStore << r;}
	void restoreRun(){run r = runStore.takeLast(); runPar = r.par;runWord = r.word; runChunk = r.chunk;}

	int maxPar(){return m_alt.count();}
	int maxWord(){return m_alt[runPar].count();}
	int maxChunk(){return m_alt[runPar][runWord].count();}
	int maxGlyph(){return m_alt[runPar][runWord][runChunk].count();}
	int maxAlt(const int& gIdx){return m_alt[runPar][runWord][runChunk][gIdx].count();}

	void setPar(const int& p = 0){runPar = p;}
	int par() const {return runPar;}

	void setWord(const int& w = 0){runWord = w;}
	int word() const{return runWord;}

	void setChunk(const int& c = 0){runChunk = c;}
	int chunk() const{return runChunk;}

	void addAlt(const int& gIndex, const int& gAlt)
	{
		if(!m_alt[runPar][runWord][runChunk][gIndex].contains(gAlt))
			m_alt[runPar][runWord][runChunk][gIndex].append(gAlt);
	}
	QList<int> alts(const int& gIndex) const { return m_alt[runPar][runWord][runChunk][gIndex]; }

	void setControl(const int& gIndex, const int& c){ m_control[runPar][runWord][runChunk][gIndex] = c; }
	int control(const int& gIndex) const {return m_control[runPar][runWord][runChunk][gIndex];}

	void setSelect(const int& gIndex, const int& s){ m_select[runPar][runWord][runChunk][gIndex] = s; }
	int select(const int& gIndex) const { return m_select[runPar][runWord][runChunk][gIndex]; }
	
	void fileWord(const QString& s){m_words[runPar][runWord] = s;}
	QString wordString()const{return m_words[runPar][runWord];}

	void fileChunk(const QString& s){m_chunks[runPar][runWord][runChunk] = s;}
	QString chunkString()const{return m_chunks[runPar][runWord][runChunk];}

	void cleanup()
	{
		QMap<int , QMap< int , QMap<int , QMap< int, QList<int> > > > > t_alt;
		QMap<int , QMap< int , QMap<int , QMap< int, int> > > > t_control;
		QMap<int , QMap< int , QMap<int , QMap< int, int> > > > t_select;
		QMap<int , QMap< int , QString> > t_words;
		QMap<int , QMap< int , QMap<int , QString> > > t_chunks;
		for(int p(0);p < m_alt.count(); ++p)
		{
			for(int w(0);w < m_alt[p].count(); ++w)
			{
				for(int c(0);c < m_alt[p][w].count(); ++c)
				{
					for(int g(0);g < m_alt[p][w][c].count(); ++g)
					{
						if(m_alt[p][w][c][g].count())
						{
							t_alt[p][w][c][g] = m_alt[p][w][c][g];
							t_control[p][w][c][g] = m_control[p][w][c][g];
							t_select[p][w][c][g] = m_select[p][w][c][g];
							t_words[p][w] = m_words[p][w];
							t_chunks[p][w][c] = m_chunks[p][w][c];
						}
					}
				}
			}
		}
		m_alt = t_alt;
		m_control = t_control;
		m_select = t_select;
		m_words = t_words;
		m_chunks = t_chunks;
	}
	
};

class FMAltContextLib : private QObject
{
	Q_OBJECT

	static FMAltContextLib * instance;
	static FMAltContextLib * that();
	FMAltContextLib();
	~FMAltContextLib();
	
	QMap<QString, FMAltContext*> cmap;
	QString current;
	
	public:
		static FMAltContext * SetCurrentContext(const QString& tid, const QString& font);
		static FMAltContext * GetCurrentContext();
		static void GetConnected(const QObject * receiver, const char * method);

	signals:
		void contextChanged();
		
};

#endif // FMALTCONTEXT_H
