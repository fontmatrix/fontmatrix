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

#include "fmsharestruct.h"
#include <QString>
#include <QRectF>
#include <QPointF>

class QGraphicsScene;
class FontItem;

/**
 Finally, we want to layout text elsewhere than _in_ the font or _in_ the view.
 And it’s "peu ou prou" forced if we really want to experiment in Fontmatrix
 I hope it will not be too much more than just copy & paste from FontItem & MainViewWidget - pm
**/

#define INFINITE 99999999L

struct Node
{
	struct Vector
	{
		Node* n;
		double distance;

		Vector() :n ( 0 ),distance ( 0.0 ) {}
		Vector ( Node* N, double D ) :n ( N ),distance ( D ) {}
		~Vector() {}
	};

	QList<Vector> nodes;
	int index;

	Node() {}
	Node ( int i ) :index ( i ) {}
	~Node() {foreach ( Vector v, nodes ) {if ( v.n ) delete v.n;}}

	bool hasNode ( int idx ) {foreach ( Vector v, nodes ) {if ( v.n->index == idx ) return true;}return false;}

	void sPath ( double dist, QList< int > curList, QList<int>& theList, double& theScore );
};

class FMLayout
{
	public:
		// Just think that the font must be set
		FMLayout ( QGraphicsScene* scene, FontItem* font );
		~FMLayout();

		virtual void doLayout ( const GlyphList& spec , double fs);

	private://methods
		/// Build a graph on node
		virtual void doGraph();
		/// Build the good list of lines
		virtual void doLines();
		/// Put lines on stage
		virtual void doDraw();

		// utils
		double distance ( int start, int end );

	private:// data
		// Argued
		QGraphicsScene* theScene;
		FontItem*	theFont;
		GlyphList theString;
		QRectF theRect;// Not really argued now, will come soon

		// built
		Node *node;
		QList<GlyphList> lines;
		QList<int> indices;

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

		void setProcessFeatures ( bool theValue )
		{
			processFeatures = theValue;
		}



		void setScript ( const QString& theValue )
		{
			script = theValue;
		}


		void setProcessScript ( bool theValue )
		{
			processScript = theValue;
		}



		void setAdjustedSampleInter ( double theValue )
		{
			adjustedSampleInter = theValue;
		}


		void setTextProgressionBlock ( int theValue )
		{
			textProgressionBlock = theValue;
		}


		void setTextProgressionLine ( int theValue )
		{
			textProgressionLine = theValue;
		}


		void setOrigine ( const QPoint& theValue )
		{
			origine = theValue;
		}

	void setFontSize ( double theValue )
	{
		fontSize = theValue;
	}
	



};


#endif

