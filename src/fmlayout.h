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

class QGraphicsScene;
class FontItem;

/**
 Finally, we want to layout text elsewhere than _in_ the font or _in_ the view.
 And it’s "peu ou prou" forced if we really want to experiment in Fontmatrix
 I hope it will not be too much more than just copy & paste from FontItem & MainViewWidget - pm
**/


struct Node
{
	struct Vector
	{
		Node* n;
		double distance;
		
		Vector():n(0),distance(0.0){}
		Vector(Node* N, double D):n(N),distance(D){}
		~Vector(){}
	};
	
	QList<Vector> nodes;
	int index;
	
	Node(){}
	Node(int i):index(i){}
	~Node(){foreach(Vector v, nodes){if(v.n)delete v.n;}}
};

class FMLayout
{
	public:
		// Just think that the font must be set
		FMLayout ( QGraphicsScene* scene, FontItem* font );
		~FMLayout();

		virtual void doLayout ( const GlyphList& spec );

	private://methods
		/// Build a graph on node
		virtual void doGraph();
		/// Build the good list of lines
		virtual void doLines();
		/// Put lines on stage
		virtual void doDraw();
		
		// utils
		double distance(int start, int end);
		
	private:// data
		// Argued
		const QGraphicsScene* theScene;
		const FontItem*	theFont;
		GlyphList theString;
		QRectF theRect;// Not really argued now, will come soon
		
		// built
		Node node;
		QList<GlyphList> lines;

		// accessed
		bool processFeatures;
		QString script;
		bool processScript;
		double adjustedSampleInter;
		int textProgressionBlock;
		int textProgressionLine;
		

	public: //accessors

		void setProcessFeatures ( bool theValue )
		{
			processFeatures = theValue;
		}


		bool getProcessFeatures() const
		{
			return processFeatures;
		}

		void setScript ( const QString& theValue )
		{
			script = theValue;
		}


		QString getScript() const
		{
			return script;
		}

		void setProcessScript ( bool theValue )
		{
			processScript = theValue;
		}


		bool getProcessScript() const
		{
			return processScript;
		}

		void setAdjustedSampleInter ( double theValue )
		{
			adjustedSampleInter = theValue;
		}


		double getAdjustedSampleInter() const
		{
			return adjustedSampleInter;
		}

		void setTextProgressionBlock ( int theValue )
		{
			textProgressionBlock = theValue;
		}


		int getTextProgressionBlock() const
		{
			return textProgressionBlock;
		}

		void setTextProgressionLine ( int theValue )
		{
			textProgressionLine = theValue;
		}


		int getTextProgressionLine() const
		{
			return textProgressionLine;
		}


};


#endif

