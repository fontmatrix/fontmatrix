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
#include "shortcuts.h"

#include <QString>
#include <QGraphicsScene>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QTime>
#include <QAction>
#include <QMenu>

Node::Node ( int i ) :index ( i )
{
}

Node::~Node()
{
// 	qDebug()<<"R"<< index;
	foreach ( Vector v, nodes )
	{
		if ( v.n ) delete v.n;
	}
}

int Node::count()
{
	int c ( nodes.count() );
	foreach ( Vector v, nodes )
	{
		c += v.n->count();
	}
	return c;
}

// #define NODE_SOON_F	20.0
// #define NODE_FIT_F	1.0
// #define NODE_LATE_F	500.0
// #define NODE_END_F	0.5



void Node::sPath ( double dist , QList< int > curList, QList< int > & theList, double & theScore )
{
// 	QList<int> debugL;
// 	foreach(Vector v, nodes)
// 	{debugL << v.n->index;}
// 	qDebug()<<"Node::sPath(" <<dist<< ", "<<curList<<", "<<theList<<", "<<theScore<<")"<< "I L"<<index<<debugL;
	int deep ( curList.count() +1 );
	FMLayout* lyt ( FMLayout::getLayout() );
	nodes.clear();
	// cIdx is first glyph of the line
	int cIdx ( index );
	// bIndex is the break which sits on cIdx
	int bIndex ( lyt->breakList.indexOf ( cIdx ) );
	bool wantPlus ( true );
	while ( wantPlus )
	{
		++bIndex;
// 		qDebug()<<"DOGRAPH LOOP "<<bIndex;
		if ( bIndex  < lyt->breakList.count() )
		{
			double di ( lyt->distance ( cIdx, lyt->breakList[bIndex] ) );
// 			qDebug()<< "C DI"<< lyt->lineWidth(deep) <<di;
			if ( di >= lyt->lineWidth ( deep ) )
			{
// 				qDebug()<<"LINE"<<cIdx<< lyt->breakList[bIndex];
// 				qDebug()<< "C DI"<< lyt->lineWidth(deep) <<di;
// 				qDebug()<< "N"<<nodes.count();
				if ( ( bIndex - 1 > 0 ) && ( lyt->breakList[bIndex - 1] != cIdx ) )
				{
					int soon ( lyt->breakList[bIndex - 1] );
					Node* sN = 0;
					sN = new Node ( soon );
					double disN = lyt->lineWidth ( deep )- lyt->distance ( cIdx, soon );
					Node::Vector vN ( sN, qAbs ( disN * lyt->FM_LAYOUT_NODE_SOON_F ) );
					if ( nodes.isEmpty() )
						nodes << vN;
					else
					{
						for ( int nI ( 0 );nI<nodes.count();++nI )
						{
							if ( vN.distance <= nodes[nI].distance )
							{
								nodes.insert ( nI,vN );
								break;
							}
						}
					}
				}

				int fit ( lyt->breakList[bIndex] );
				Node* sF = 0;
				sF = new Node ( fit );
				double disF =  lyt->lineWidth ( deep )- lyt->distance ( cIdx, fit );
				Node::Vector vF ( sF,qAbs ( disF * lyt->FM_LAYOUT_NODE_FIT_F ) );
// 				curNode->nodes << vF;
				if ( nodes.isEmpty() )
					nodes << vF;
				else
				{
					for ( int nI ( 0 );nI<nodes.count();++nI )
					{
						if ( vF.distance <= nodes[nI].distance )
						{
							nodes.insert ( nI,vF );
							break;
						}
					}
				}

				if ( bIndex + 1 <  lyt->breakList.count() )
				{
					int late ( lyt->breakList[bIndex + 1] );
					Node* sL = 0;
					sL = new Node ( late );
					double disL = lyt->lineWidth ( deep )- lyt->distance ( cIdx, late );
					Node::Vector vL ( sL, qAbs ( disL * lyt->FM_LAYOUT_NODE_LATE_F ) );
					if ( nodes.isEmpty() )
						nodes << vL;
					else
					{
						for ( int nI ( 0 );nI<nodes.count();++nI )
						{
							if ( vL.distance <= nodes[nI].distance )
							{
								nodes.insert ( nI,vL );
								break;
							}
						}
					}
				}

				wantPlus = false;

			}
		}
		else // end of breaks list
		{
// 					qDebug()<<"END OF BREAKS";
			int soon ( lyt->breakList[bIndex - 1] );
			if ( soon != cIdx && !hasNode ( soon ) )
			{

				Node* sN = 0;
				sN = new Node ( soon );
				double disN = lyt->lineWidth ( deep ) - lyt->distance ( cIdx, soon );
				Node::Vector vN ( sN, qAbs ( disN * lyt->FM_LAYOUT_NODE_END_F ) );
// 				curNode->nodes << vN;
				if ( nodes.isEmpty() )
					nodes << vN;
				else
				{
					for ( int nI ( 0 );nI<nodes.count();++nI )
					{
						if ( vN.distance <= nodes[nI].distance )
							nodes.insert ( nI,vN );
					}
				}
			}

			wantPlus = false;
		}
	}

// 	qDebug()<<"N"<<nodes.count();
	curList << index;
	bool isLeaf ( nodes.isEmpty() );
	while ( !nodes.isEmpty() )
	{
		Vector v = nodes.first() ;
		if ( dist + v.distance < theScore )
		{
			v.n->sPath ( dist + v.distance, curList, theList, theScore );
		}
// 		qDebug()<< "R";
		delete v.n;
		nodes.removeFirst();
	}

	if ( isLeaf )
	{
		if ( dist < theScore )
		{
			theScore = dist;
			theList = curList;
		}
	}

}

FMLayout *FMLayout::instance = 0;
FMLayout::FMLayout ( /*QGraphicsScene * scene, FontItem * font */ )
{
	rules = new QGraphicsRectItem;
	instance = this;
	node = 0;

	FM_LAYOUT_NODE_SOON_F=	1.25;
	FM_LAYOUT_NODE_FIT_F=	1.0;
	FM_LAYOUT_NODE_LATE_F=	500.0;
	FM_LAYOUT_NODE_END_F=	0.5;

	soonplus = new QAction ( "soonplus",this );
	Shortcuts::getInstance()->add ( soonplus );
	connect ( soonplus,SIGNAL ( triggered() ),this, SLOT ( slotSP() ) );
	soonmoins = new QAction ( "soonmoins",this );
	Shortcuts::getInstance()->add ( soonmoins );
	connect ( soonmoins,SIGNAL ( triggered() ),this, SLOT ( slotSM() ) );
	fitplus = new QAction ( "fitplus",this );
	Shortcuts::getInstance()->add ( fitplus );
	connect ( fitplus,SIGNAL ( triggered() ),this, SLOT ( slotFP() ) );
	fitmoins = new QAction ( "fitmoins",this );
	Shortcuts::getInstance()->add ( fitmoins );
	connect ( fitmoins,SIGNAL ( triggered() ),this, SLOT ( slotFM() ) );
	lateplus = new QAction ( "lateplus",this );
	Shortcuts::getInstance()->add ( lateplus );
	connect ( lateplus,SIGNAL ( triggered() ),this, SLOT ( slotLP() ) );
	latemoins = new QAction ( "latemoins",this );
	Shortcuts::getInstance()->add ( latemoins );
	connect ( latemoins,SIGNAL ( triggered() ),this, SLOT ( slotLM() ) );
	endplus = new QAction ( "endplus",this );
	Shortcuts::getInstance()->add ( endplus );
	connect ( endplus,SIGNAL ( triggered() ),this, SLOT ( slotEP() ) );
	endmoins = new QAction ( "endmoins",this );
	Shortcuts::getInstance()->add ( endmoins );
	connect ( endmoins,SIGNAL ( triggered() ),this, SLOT ( slotEM() ) );
	secretMenu = new QMenu(0);
	secretMenu->addAction(soonplus);
	secretMenu->addAction(soonmoins);
	secretMenu->addAction(fitplus);
	secretMenu->addAction(fitmoins);
	secretMenu->addAction(lateplus);
	secretMenu->addAction(latemoins);
	secretMenu->addAction(endplus);
	secretMenu->addAction(endmoins);
	
}

FMLayout::~ FMLayout()
{
}

void FMLayout::doLayout ( const QList<GlyphList> & spec , double fs )
{
	qDebug() <<"FMLayout::doLayout()";
	fontSize = fs;
	resetScene();
	for ( int i ( 0 ); i < spec.count() ; ++ i )
	{
		theString = spec[i];
		node = new Node ( 0 );
		doGraph();
		doLines();
		doDraw();
// 		qDebug()<<"N"<<node->count();
		delete node;
	}
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
}

void FMLayout::doGraph() // Has became doBreaks
{
	qDebug() <<"FMLayout::doGraph()";
	QTime t;
	t.start();
	/**
	I hit a power issue with my graph thing as in its initial state.
	So, I’ll try now to cut the tree as it’s built, not far than what’s done in chess programs.

	*/
	// At first we’ll provide a very simple implementation to test things out

	// A) where can we break?
	breakList.clear();
	for ( int a ( 0 ) ; a < theString.count() ; ++a )
	{
		// with the simple we just break at space, I know it’s ugly
// 		qDebug() << theString[a].lChar<< QChar(theString[a].lChar);
		if ( QChar ( theString[a].lChar ).category() == QChar::Separator_Space )
			breakList << a;
	}
	breakList << theString.count();
	qDebug() <<"BREAKS"<<breakList.count();
	qDebug() <<"doGraph T(ms)"<<t.elapsed();
}

void FMLayout::doLines()
{
	qDebug() <<"FMLayout::doLines()";
	QTime t;
	t.start();
	// Run through the graph and find the shortest path
	indices.clear();
	double score ( INFINITE );
	node->sPath ( 0,QList<int>(),indices,score );
	// la messe est dite ! :-)
	qDebug() <<"S I"<<score<<indices;
	double refW ( theRect.width() );
	for ( int lIdx ( 1 ); lIdx<indices.count(); ++lIdx )
	{
// 		bool leadWS(true);
		for ( int ri ( indices[lIdx-1] );ri < indices[lIdx]; ++ri )
		{
			if ( QChar ( theString.at ( ri ).lChar ).category() == QChar::Separator_Space )
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
		for ( int ri ( indices[lIdx] - 1 );ri > indices[lIdx - 1]; --ri )
		{
			if ( QChar ( theString.at ( ri ).lChar ).category() == QChar::Separator_Space )
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
		double diff ( refW - distance ( indices[lIdx-1], indices[lIdx] ) );
		for ( int ri ( indices[lIdx-1] );ri < indices[lIdx]; ++ri )
		{
			if ( theString.at ( ri ).glyph &&  QChar ( theString.at ( ri ).lChar ).category() == QChar::Separator_Space )
			{
				wsIds << ri;
			}
		}
		double shareLost ( diff / qMax ( 1.0 , ( double ) wsIds.count() ) );
// 		qDebug()<< "D N W"<<diff<<wsIds.count()<< shareLost;
		for ( int wi ( 0 ); wi < wsIds.count(); ++wi )
		{
			theString[ wsIds[wi] ].xadvance += shareLost;
		}
	}
	qDebug() <<"doLines T(ms)"<<t.elapsed();
}

void FMLayout::doDraw()
{
	// Ask paths or pixmaps to theFont for each glyph and draw it on theScene
	qDebug() <<"FMLayout::doDraw()";
	QTime t;
	t.start();
	QPointF pen ( origine );
	pen.ry() += adjustedSampleInter;
	double pageBottom ( theRect.bottom() );
	double scale = fontSize / theFont->getUnitPerEm();
	double pixelAdjustX = ( double ) QApplication::desktop()->physicalDpiX() / 72.0 ;
	double pixelAdjustY = ( double ) QApplication::desktop()->physicalDpiX() / 72.0 ;
	int m_progression ( theFont->progression() );

	for ( int lIdx ( 1 ); lIdx<indices.count(); ++lIdx )
	{
		if ( pen.y() > pageBottom )
			break;
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
				if ( !refGlyph[i].glyph )
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
				if ( !refGlyph[i].glyph )
					continue;
				QGraphicsPathItem *glyph = theFont->itemFromGindex ( refGlyph[i].glyph , fontSize );
				glyphList << glyph;

				if ( m_progression == PROGRESSION_RTL )
				{
					pen.rx() -= refGlyph[i].xadvance ;
				}
				else if ( m_progression == PROGRESSION_BTT )
				{
					pen.ry() -= refGlyph[i].yadvance ;
				}
				/**********************************************/

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

		origine.ry() = pen.y();
		pen.ry() += adjustedSampleInter;
		pen.rx() = origine.x() ;
	}

	qDebug() <<"doDraw T(ms)"<<t.elapsed();
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
	qDebug() <<"FMLayout::resetScene(P"<<pixList.count() <<",G"<<glyphList.count() <<")";
	QTime t;
	t.start();
	int pCount ( pixList.count() );
	for ( int i = 0; i < pCount ; ++i )
	{
		if ( pixList[i]->scene() )
		{
			pixList[i]->scene()->removeItem ( pixList[i] );
		}
		delete pixList[i];
	}
	pixList.clear();
	int gCount ( glyphList.count() );
	for ( int i = 0; i < gCount; ++i )
	{
		if ( glyphList[i]->scene() )
		{
			glyphList[i]->scene()->removeItem ( glyphList[i] );
		}
		delete glyphList[i];
	}
	glyphList.clear();
	qDebug() <<"resetScene T(ms)"<<t.elapsed();
}





void FMLayout::setTheScene ( QGraphicsScene* theValue )
{
	theScene = theValue;
	QRectF tmpRect = theScene->sceneRect();
	double sUnitW ( tmpRect.width() * .1 );
	double sUnitH ( tmpRect.height() * .1 );

	theRect.setX ( 2.0 * sUnitW );
	theRect.setY ( 2.0 * sUnitH );
	theRect.setWidth ( 6.0 * sUnitW );
	theRect.setHeight ( 6.0 * sUnitH );
	origine = theRect.topLeft() ;
// 	rules->setRect(theRect);
// 	rules->setZValue(9.9);
// 	if(rules->scene() != theScene)
// 		theScene->addItem(rules);
}


void FMLayout::setTheFont ( FontItem* theValue )
{
	theFont = theValue;
}

double FMLayout::lineWidth ( int l )
{
	return theRect.width();
}


void FMLayout::slotSP() {
	FM_LAYOUT_NODE_SOON_F *= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
emit updateLayout();}
void FMLayout::slotSM() {
	FM_LAYOUT_NODE_SOON_F /= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
	emit updateLayout();}
void FMLayout::slotFP() {
	FM_LAYOUT_NODE_FIT_F *= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
	emit updateLayout();}
void FMLayout::slotFM() {
	FM_LAYOUT_NODE_FIT_F /= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
	emit updateLayout();}
void FMLayout::slotLP() {
	FM_LAYOUT_NODE_LATE_F *= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
	emit updateLayout();}
void FMLayout::slotLM() {
	FM_LAYOUT_NODE_LATE_F /= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
	emit updateLayout();}
void FMLayout::slotEP() {
	FM_LAYOUT_NODE_END_F *= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
	emit updateLayout();}
void FMLayout::slotEM() {
	FM_LAYOUT_NODE_END_F /= 2.0;
	qDebug() <<"LF"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F;
	emit updateLayout();}
