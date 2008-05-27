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
#include "fontitem.h"

#include <QString>
#include <QGraphicsScene>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

void Node::sPath ( double dist , QList< int > curList, QList< int > & theList, double & theScore )
{
// 	QList<int> debugL;
// 	foreach(Vector v, nodes)
// 	{debugL << v.n->index;}
// 	qDebug()<<"Node::sPath(" <<dist<< ", "<<curList<<", "<<theList<<", "<<theScore<<")"<< "I L"<<index<<debugL;
	curList << index;
	foreach ( Vector v, nodes )
	{
		if ( dist + v.distance < theScore )
		{
			v.n->sPath ( dist + v.distance, curList, theList, theScore );
		}
	}

	if ( nodes.isEmpty() ) // end of text
	{
		if ( dist < theScore )
		{
			theScore = dist;
			theList = curList;
		}
	}

}


FMLayout::FMLayout ( /*QGraphicsScene * scene, FontItem * font */)
{
	rules = new QGraphicsRectItem;
}

FMLayout::~ FMLayout()
{
}

void FMLayout::doLayout ( const GlyphList & spec , double fs )
{
	fontSize = fs;
	theString = spec;
	node = new Node ( 0 );
	resetScene();
	doGraph();
	doLines();
	doDraw();
	delete node;
}

void FMLayout::doGraph()
{
	qDebug() <<"FMLayout::doGraph()";
	// At first we’ll provide a very simple implementation to test things out

	// A) where can we break?
	QList<int> breaks;
// 	breaks << 0;
	for ( int a ( 0 ) ; a < theString.count() ; ++a )
	{
		// with the simple we just break at space, I know it’s ugly
// 		qDebug() << theString[a].lChar<< QChar(theString[a].lChar);
		if ( QChar ( theString[a].lChar ).category() == QChar::Separator_Space )
			breaks << a;
	}
	breaks << theString.count();
	qDebug() <<"BREAKS"<<breaks;
	int theEnd ( breaks.last() );
// 	return;

	// B) Try to build the shorter graph (be carefull of n) - we’ll try, if possible, to avoid recursive function
	double constantWidth ( theRect.width() );// here it’s constant but it will have to not be in the future

// 	QMap<int,Node*> crossBreakNode;
// 	QMap<int,int> crossBreakVectIndex;
	QList<Node*> toDo;
	QList<Node*> newNodes;
	toDo << node;
	int nodeCounter ( 0 );
	while ( !toDo.isEmpty() )
	{
// 		qDebug()<<"DOGRAPH LOOP 1";
		newNodes.clear();
		int nCount ( toDo.count() );
		for ( int nIdx ( 0 ); nIdx < nCount; ++nIdx )
		{
// 			qDebug()<<"DOGRAPH LOOP 2"<<nIdx <<"/"<<nCount;
			Node* curNode ( toDo[nIdx] );
			// cIdx is first glyph of the line
			int cIdx ( curNode->index );
			// bIndex is the break which sits on cIdx
			int bIndex ( breaks.indexOf ( cIdx ) );
			bool wantPlus ( true );
			while ( wantPlus )
			{
				++bIndex;
// 				qDebug()<<"DOGRAPH LOOP 3"<<plus;
				if ( bIndex  < breaks.count() )
				{
					double di ( distance ( cIdx, breaks[bIndex] ) );
// 					qDebug()<< "C DI"<< constantWidth<<di;
					if ( di >= constantWidth )
					{
// 						qDebug()<<"LINE"<<cIdx<< breaks[bIndex];
// 						qDebug()<<"C DI"<< constantWidth<<di;
						if ( ( bIndex - 1 > 0 ) && ( breaks[bIndex - 1] != cIdx ) )
						{
							int soon ( breaks[bIndex - 1] );
							Node* sN = 0;
							sN = new Node ( soon );
							++nodeCounter;
// 							}
							double disN = constantWidth - distance ( cIdx, soon );
							Node::Vector vN ( sN, qAbs ( disN ) );
							curNode->nodes << vN;
							newNodes << sN;
						}

						int fit ( breaks[bIndex] );
						Node* sF = 0;
						sF = new Node ( fit );
						++nodeCounter;
// 						}
						double disF = constantWidth - distance ( cIdx, fit );
						Node::Vector vF ( sF,qAbs (  2.0 * disF ) );
						curNode->nodes << vF;
						newNodes << sF;

						wantPlus = false;

					}
				}
				else // end of breaks list
				{
// // 					qDebug()<<"END OF BREAKS";
					int soon ( breaks[bIndex - 1] );
					if ( soon != cIdx && !curNode->hasNode ( soon ) )
					{

						Node* sN = 0;
						sN = new Node ( soon );
						++nodeCounter;
// 						}
						double disN = constantWidth - distance ( cIdx, soon );
						Node::Vector vN ( sN, qAbs ( disN ) );
						curNode->nodes << vN;
						newNodes << sN;
					}

					wantPlus = false;
				}
			}

		}
		toDo.clear();
		toDo = newNodes;
	}
	qDebug() <<nodeCounter<<"Nodes created";

}

void FMLayout::doLines()
{
	qDebug() <<"FMLayout::doLines()";
	// Run through the graph and find the shortest path
	indices.clear();
	double score ( INFINITE );
	node->sPath ( 0,QList<int>(),indices,score );
	// la messe est dite ! :-)
	qDebug() <<"S I"<<score<<indices;
	double refW(theRect.width());
	for ( int lIdx ( 1 ); lIdx<indices.count(); ++lIdx )
	{
// 		bool leadWS(true);
		for ( int ri ( indices[lIdx-1] );ri < indices[lIdx]; ++ri )
		{
			if( QChar(theString.at(ri).lChar).category() == QChar::Separator_Space)
			{
				theString[ri].glyph = 0;
				theString[ri].xadvance = 0;
				theString[ri].xoffset = 0;
			}
			else
			{
				break;
			}
			
		}
// 		bool trailingWS(false);
		for ( int ri ( indices[lIdx] - 1);ri > indices[lIdx - 1]; --ri )
		{
			if( QChar(theString.at(ri).lChar).category() == QChar::Separator_Space)
			{
				theString[ri].glyph = 0;
				theString[ri].xadvance = 0;
				theString[ri].xoffset = 0;
			}
			else
			{
				break;
			}
			
		}
	}
	for ( int lIdx ( 1 ); lIdx<indices.count() - 1; ++lIdx )
	{
		QList<int> wsIds;
		double diff( refW - distance(indices[lIdx-1], indices[lIdx]) );
		for ( int ri ( indices[lIdx-1] );ri < indices[lIdx]; ++ri )
		{
			if(theString.at(ri).glyph &&  QChar(theString.at(ri).lChar).category() == QChar::Separator_Space)
			{
				wsIds << ri;
			}			
		}
		double shareLost( diff / qMax( 1.0 , (double)wsIds.count() ) );
		qDebug()<< "D N W"<<diff<<wsIds.count()<< shareLost;
		for(int wi(0); wi < wsIds.count(); ++wi)
		{
			theString[ wsIds[wi] ].xadvance += shareLost;
		}
	}
}

void FMLayout::doDraw()
{
	// Ask paths or pixmaps to theFont for each glyph and draw it on theScene
	QPointF pen ( origine );
	pen.ry() += adjustedSampleInter;
	double scale = fontSize / theFont->getUnitPerEm();
	double pixelAdjustX = ( double ) QApplication::desktop()->physicalDpiX() / 72.0 ;
	double pixelAdjustY = ( double ) QApplication::desktop()->physicalDpiX() / 72.0 ;
	int m_progression ( theFont->progression() );
	
	for ( int lIdx ( 1 ); lIdx<indices.count(); ++lIdx )
	{
		GlyphList refGlyph;
		for ( int ri ( indices[lIdx-1] );ri < indices[lIdx]; ++ri )
		{
			refGlyph << theString.at ( ri );
// 			qDebug() <<"A["<< ri <<"]"<<theString.at ( ri ).glyph;
		}

		if ( theFont->rasterFreetype() )
		{
			for ( int i=0; i < refGlyph.count(); ++i )
			{
				if(!refGlyph[i].glyph)
					continue;
				QGraphicsPixmapItem *glyph = theFont->itemFromGindexPix ( refGlyph[i].glyph , fontSize );
				if ( !glyph )
					continue;
				if ( m_progression == PROGRESSION_RTL )
				{
					pen.rx() -= refGlyph[i].xadvance * pixelAdjustX ;
				}
				else if ( m_progression == PROGRESSION_BTT )
				{
					pen.ry() -= refGlyph[i].yadvance * pixelAdjustY;
				}

				/*************************************************/
				pixList << glyph;
				theScene->addItem ( glyph );
				glyph->setZValue ( 100.0 );
				glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * pixelAdjustX ) + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() * scale  ,
				                pen.y() + ( refGlyph[i].yoffset * pixelAdjustY ) - glyph->data ( GLYPH_DATA_BITMAPTOP ).toInt() );
				/*************************************************/

				if ( m_progression == PROGRESSION_LTR )
					pen.rx() += refGlyph[i].xadvance * pixelAdjustX ;
				else if ( m_progression == PROGRESSION_TTB )
					pen.ry() += refGlyph[i].yadvance * pixelAdjustY ;
			}
		}
		else
		{
			for ( int i=0; i < refGlyph.count(); ++i )
			{
				if(!refGlyph[i].glyph)
					continue;
				QGraphicsPathItem *glyph = theFont->itemFromGindex ( refGlyph[i].glyph , fontSize );

				if ( m_progression == PROGRESSION_RTL )
				{
					pen.rx() -= refGlyph[i].xadvance ;
				}
				else if ( m_progression == PROGRESSION_BTT )
				{
					pen.ry() -= refGlyph[i].yadvance ;
				}
				/**********************************************/
				glyphList << glyph;
				theScene->addItem ( glyph );
				glyph->setPos ( pen.x() + ( refGlyph[i].xoffset ),
				                pen.y() + ( refGlyph[i].yoffset ) );
				glyph->setZValue ( 100.0 );
				/*******************************************/

				if ( m_progression == PROGRESSION_LTR )
					pen.rx() += refGlyph[i].xadvance ;
				if ( m_progression == PROGRESSION_TTB )
					pen.ry() += refGlyph[i].yadvance;

			}
		}

		pen.ry() += adjustedSampleInter;
		pen.rx() = origine.x();
	}
}

double FMLayout::distance ( int start, int end )
{
// 	qDebug()<<"distance("<<start<<", "<< end<<")";
	double ret ( 0.0 );
	if ( end <= start )
		return ret;
	for ( int i ( start ); i < end ;++i )
		ret += theString.at ( i ).xadvance /*+ theString.at ( i ).xoffset*/;

	return ret;
}

void FMLayout::resetScene()
{
	for ( int i = 0; i < pixList.count(); ++i )
	{
		if ( pixList[i]->scene() )
		{
			pixList[i]->scene()->removeItem ( pixList[i] );
			delete pixList[i];
		}
	}
	pixList.clear();
	for ( int i = 0; i < glyphList.count(); ++i )
	{
		if ( glyphList[i]->scene() )
		{
			glyphList[i]->scene()->removeItem ( glyphList[i] );
			delete glyphList[i];
		}
	}
	glyphList.clear();
}





void FMLayout::setTheScene ( QGraphicsScene* theValue )
{
	theScene = theValue;
	QRectF tmpRect = theScene->sceneRect();
	double sUnitW(tmpRect.width() * .1);
	double sUnitH(tmpRect.height() * .1);
	
	theRect.setX ( 2.0 * sUnitW );
	theRect.setY ( 2.0 * sUnitH );
	theRect.setWidth( 6.0 * sUnitW );
	theRect.setHeight ( 6.0 * sUnitH );
	origine = theRect.topLeft() ;
	rules->setRect(theRect);
	rules->setZValue(9.9);
	if(rules->scene() != theScene)
		theScene->addItem(rules);
}


void FMLayout::setTheFont ( FontItem* theValue )
{
	theFont = theValue;
}
