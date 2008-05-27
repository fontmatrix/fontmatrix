//
// C++ Implementation: fmlayout
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fmlayout.h"

#include <QString>
#include <QGraphicsScene>
#include <QDebug>

FMLayout::FMLayout(QGraphicsScene * scene, FontItem * font)
	:theScene(scene), theFont(font)
{
	theRect = theScene->sceneRect();
	theRect.setWidth(theRect.width() *.8);
	theRect.setHeight(theRect.height() * .8);
	
}

FMLayout::~ FMLayout()
{
}

void FMLayout::doLayout(const GlyphList & spec)
{
	theString = spec;
	
	doGraph();
}

void FMLayout::doGraph()
{
	// At first we’ll provide a very simple implementation to test things out
	
	// A) where can we break?
	QList<int> breaks;
	breaks << 0;
	for(int a(0) ; a < theString.count() ; ++a)
	{  
		// with the simple we just break at space, I know it’s ugly
		if(QChar(theString[a].lChar).category() == QChar::Separator_Space)
			breaks << a;
	}
	breaks << theString.count();
	
	// B) Try to build the shorter graph (be carefull of n) - we’ll try, if possible, to avoid recursive function
	double constantWidth(theRect.width());// here it’s constant but it will have to not be in the future
	node.index = 0;
	QList<Node*> toDo;
	QList<Node*> newNodes;
	toDo << &node;
	while(!toDo.isEmpty())
	{		
		newNodes.clear();
		int nCount(toDo.count());
		for(int nIdx(0); nIdx < nCount; ++nIdx)
		{
			Node* curNode(toDo[nIdx]);
			int cIdx(curNode->index);
			int bIndex( breaks.indexOf(cIdx) );
			QList<double> distances;
			int plus(1);
			while(plus > 0)
			{
				if( bIndex + plus < breaks.count() )
				{
					double di(distance(cIdx, breaks[bIndex + plus]));
					if(di >= constantWidth)
					{
						int soon(breaks[bIndex + plus - 1]);
						double disN = constantWidth - distance(cIdx, soon);
						Node* sN = new Node(soon);
						Node::Vector vN(sN, disN);
						curNode->nodes << vN;
						newNodes << sN;
						
						int fit(breaks[bIndex + plus]);
						double disF = constantWidth - distance(cIdx, fit);
						Node* sF = new Node(fit);
						Node::Vector vF(sF,disF);
						curNode->nodes << vF;
						newNodes << sF;
						
						int late(breaks[bIndex + plus + 1]);
						double disL = constantWidth - distance(cIdx, late);
						Node* sL = new Node(late);
						Node::Vector vL(sL,disL);
						curNode->nodes << vL;
						newNodes << sL;
						
						plus = 0;
					}
					else
						++plus;
				}
				else // end of breaks list
				{
					int soon(breaks[bIndex + plus - 1]);
					double disN = constantWidth - distance(cIdx, soon);
					Node* sN = new Node(soon);
					Node::Vector vN(sN, disN);
					curNode->nodes << vN;
// 					newNodes << sN;
					plus = 0;
				}
			}
			
		} 
		toDo.clear();
		toDo = newNodes;
	}
	
}

void FMLayout::doLines()
{
}

void FMLayout::doDraw()
{
}

double FMLayout::distance(int start, int end)
{
	double ret(0);
	if(end < start)
		return ret;
	for(int i(start); i <= end ;++i)
		ret += theString.at(i).xadvance + theString.at(i).xoffset;
	return ret;
}


