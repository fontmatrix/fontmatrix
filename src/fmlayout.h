//
// C++ Interface: fmlayout
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMLAYOUT_H
#define FMLAYOUT_H


#include <QObject>
#include <QString>
#include <QRectF>
#include <QPointF>
#include <QMap>
#include <QThread>

#include "fmsharestruct.h"

class QGraphicsScene;
class FontItem;
class QGraphicsPixmapItem;
class QGraphicsPathItem;
class QGraphicsRectItem;
class QGraphicsProxyWidget;
class QProgressBar;
class QAction;
class QMenu;
class QMutex;
class FMLayOptWidget;
class QDialog;

/**
 Finally, we want to layout text elsewhere than _in_ the font or _in_ the view.
 And it’s "peu ou prou" forced if we really want to experiment in Fontmatrix
 I hope it will not be too much more than just copy & paste from FontItem & MainViewWidget - pm
 It has been quite more! - pm
**/

#define INFINITE 99999999L




struct Node
{
	struct Vector
	{
		Node* n;
		double distance;
		
		
		Vector();
		Vector ( Node* N, double D );
		~Vector() ;
// 		private:
// 		Vector (const Vector& v);
	};

	
	QList<Vector> nodes;
	int index;

	
	Node() {}
	Node ( int i ) ;
	~Node();

	bool hasNode ( int idx ) ;
// 	void nodes_insert(int idx, Vector& v);

	void sPath ( double dist, QList< int > curList, QList<int>& theList, double& theScore );
	int count();
	
};


		
class FMLayout : public QThread
{
	Q_OBJECT
	public:
		// Just think that the font must be set
		FMLayout ( /*QGraphicsScene* scene, FontItem* font*/ );
		~FMLayout();

		void doLayout(const QList<GlyphList>& spec , double fs);


		static FMLayout *instance;
		static FMLayout *getLayout() {return instance;}
	private://methods
		/// Build a graph on node
		virtual void doGraph();
		/// Build the good list of lines
		virtual void doLines();
		
		void run();

	public:// utils
		double distance ( int start, int end, const GlyphList& gl , bool power = false );
		void resetScene();

		QList<int> breakList;
		QList<int> hyphenList;
		GlyphList theString;
		double lineWidth ( int l );
		QMutex *layoutMutex;
		bool stopIt;
		
	public slots:
		void stopLayout();		

	private:// data
		// Argued
		QGraphicsScene* theScene;
		FontItem*	theFont;
		QList<GlyphList> paragraphs;
		QList<GlyphList> lines;
		QRectF theRect;// Not really argued now, will come soon
		QGraphicsRectItem *rules;
		QGraphicsProxyWidget *onSceneProgressBar;
		QProgressBar *progressBar;
		FMLayOptWidget *optionsWidget;

		// built
		Node *node;
		QList<int> indices;
		QList<QGraphicsPixmapItem *> pixList;
		QList<QGraphicsPathItem*> glyphList;
		QMap<int, QMap<int, double > > distCache;
		

		// accessed
		bool processFeatures;
		QString script;
		bool processScript;
		double fontSize;
		double adjustedSampleInter;
		int textProgressionBlock;
		int textProgressionLine;
		QPointF origine;


	public: //accessors
		QRectF getRect()const{return theRect;}
		void setProcessFeatures ( bool theValue ){processFeatures = theValue;}
		void setScript ( const QString& theValue ){script = theValue;}
		void setProcessScript ( bool theValue )	{processScript = theValue;}
		void setAdjustedSampleInter ( double theValue ){adjustedSampleInter = theValue;}
		void setTextProgressionBlock ( int theValue ){textProgressionBlock = theValue;}
		void setTextProgressionLine ( int theValue ){textProgressionLine = theValue;}
		void setOrigine ( const QPoint& theValue ){origine = theValue;}
		void setFontSize ( double theValue ){fontSize = theValue;}
		void setTheScene ( QGraphicsScene* theValue );
		void setTheFont ( FontItem* theValue );


	private slots:
		/// Put lines on stage
		void doDraw();
		void endOfRun();
		
		void slotOption(int v);

	signals:
		void updateLayout();
		void layoutFinished();
		void paragraphFinished(int);
	public:
		QDialog *optionDialog;
		
		double FM_LAYOUT_NODE_SOON_F;
		double FM_LAYOUT_NODE_FIT_F;
		double FM_LAYOUT_NODE_LATE_F;
		double FM_LAYOUT_NODE_END_F;
		double FM_LAYOUT_HYPHEN_PENALTY;
		QMenu * secretMenu;


};


#endif

