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

#define OUT_OF_RECT 99999999.0

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


void Node::sPath ( double dist , QList< int > curList, QList< int > & theList, double & theScore )
{
// 	QList<int> debugL;
// 	foreach(Vector v, nodes)
// 	{debugL << v.n->index;}
// 	qDebug()<<"Node::sPath(" <<dist<< ", "<<curList<<", "<<theList<<", "<<theScore<<")"<< "I L"<<index<<debugL;
	int deep ( curList.count() + 1 );
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
			double di ( lyt->distance ( cIdx, lyt->breakList[bIndex],lyt->theString ) );
// 			qDebug()<< "C DI"<< lyt->lineWidth(deep) <<di;
			if ( di >= lyt->lineWidth ( deep ) )
			{
// 				qDebug()<<"LINE"<<cIdx<< lyt->breakList[bIndex];
// 				qDebug()<< "C DI"<< lyt->lineWidth(deep) <<di;
// 				qDebug()<< "N"<<nodes.count();
				for (int backIndex(1) ;( bIndex - backIndex > 0 ) && ( lyt->breakList[bIndex - backIndex] != cIdx ); ++backIndex)
				{
					int soon ( lyt->breakList[bIndex - backIndex] );
					Node* sN = 0;
					sN = new Node ( soon );
					double disN = lyt->lineWidth ( deep )- lyt->distance ( cIdx, soon,lyt->theString );
					if(lyt->hyphenList.contains(soon))
						disN *= lyt->FM_LAYOUT_HYPHEN_PENALTY; 
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
					if(QChar(lyt->theString[soon].lChar).category() == QChar::Separator_Space)
						break;
				}

				int fit ( lyt->breakList[bIndex] );
				Node* sF = 0;
				sF = new Node ( fit );
				double disF =  lyt->lineWidth ( deep )- lyt->distance ( cIdx, fit,lyt->theString );
				if(lyt->hyphenList.contains(fit))
					disF *= lyt->FM_LAYOUT_HYPHEN_PENALTY; 
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

				for (int nextIndex(1); bIndex + nextIndex <  lyt->breakList.count() ; ++nextIndex)
				{
					int late ( lyt->breakList[bIndex + nextIndex] );
					Node* sL = 0;
					sL = new Node ( late );
					double disL = lyt->lineWidth ( deep )- lyt->distance ( cIdx, late ,lyt->theString);
					if(lyt->hyphenList.contains(late))
						disL *= lyt->FM_LAYOUT_HYPHEN_PENALTY; 
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
					if( late < lyt->theString.count() && QChar(lyt->theString[late].lChar).category() == QChar::Separator_Space)
						break;
				}

				wantPlus = false;

			}
			else if(lyt->lineWidth ( deep ) == OUT_OF_RECT )
			{
				wantPlus = false;
			}
		}
		else // end of breaks list
		{
// 			qDebug()<<"END OF BREAKS";
			int soon ( lyt->breakList[bIndex - 1] );
			if ( soon != cIdx && !hasNode ( soon ) )
			{

				Node* sN = 0;
				sN = new Node ( soon );
				double disN = lyt->lineWidth ( deep ) - lyt->distance ( cIdx, soon,lyt->theString );
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
	bool isLeaf ( nodes.isEmpty() );
	curList << index ;//(isLeaf ? index-1 : index);
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
		double mDist(dist * dist / deep);
		double mScore(theScore * theScore / theList.count());
		qDebug()<<"D S N"<< mDist << mScore << curList;
		if ( mDist < mScore )
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

	FM_LAYOUT_NODE_SOON_F=	625.0;
	FM_LAYOUT_NODE_FIT_F=	1000.0;
	FM_LAYOUT_NODE_LATE_F=	2000.0;
	FM_LAYOUT_NODE_END_F=	500.0;
	FM_LAYOUT_HYPHEN_PENALTY = 1.2;

	soonplus = new QAction ( "increase before",this );
	Shortcuts::getInstance()->add ( soonplus );
	connect ( soonplus,SIGNAL ( triggered() ),this, SLOT ( slotSP() ) );
	soonmoins = new QAction ( "decrease before",this );
	Shortcuts::getInstance()->add ( soonmoins );
	connect ( soonmoins,SIGNAL ( triggered() ),this, SLOT ( slotSM() ) );
	fitplus = new QAction ( "increase at break",this );
	Shortcuts::getInstance()->add ( fitplus );
	connect ( fitplus,SIGNAL ( triggered() ),this, SLOT ( slotFP() ) );
	fitmoins = new QAction ( "decrease at break",this );
	Shortcuts::getInstance()->add ( fitmoins );
	connect ( fitmoins,SIGNAL ( triggered() ),this, SLOT ( slotFM() ) );
	lateplus = new QAction ( "increase after",this );
	Shortcuts::getInstance()->add ( lateplus );
	connect ( lateplus,SIGNAL ( triggered() ),this, SLOT ( slotLP() ) );
	latemoins = new QAction ( "decrease after",this );
	Shortcuts::getInstance()->add ( latemoins );
	connect ( latemoins,SIGNAL ( triggered() ),this, SLOT ( slotLM() ) );
	endplus = new QAction ( "increase last line",this );
	Shortcuts::getInstance()->add ( endplus );
	connect ( endplus,SIGNAL ( triggered() ),this, SLOT ( slotEP() ) );
	endmoins = new QAction ( "decrease last line",this );
	Shortcuts::getInstance()->add ( endmoins );
	connect ( endmoins,SIGNAL ( triggered() ),this, SLOT ( slotEM() ) );
	hyphenpenaltyplus = new QAction("Increase hyphen penalty",this);
	Shortcuts::getInstance()->add ( hyphenpenaltyplus );
	connect ( hyphenpenaltyplus,SIGNAL ( triggered() ),this, SLOT ( slotHP() ) );
	hyphenpenaltymoins= new QAction("Decrease hyphen penalty",this);
	Shortcuts::getInstance()->add ( hyphenpenaltymoins );
	connect ( hyphenpenaltymoins,SIGNAL ( triggered() ),this, SLOT ( slotHM() ) );
	
	
	secretMenu = new QMenu("TLE node weights",0);
	secretMenu->addAction(soonplus);
	secretMenu->addAction(soonmoins);
	secretMenu->addAction(fitplus);
	secretMenu->addAction(fitmoins);
	secretMenu->addAction(lateplus);
	secretMenu->addAction(latemoins);
	secretMenu->addAction(endplus);
	secretMenu->addAction(endmoins);
	secretMenu->addAction(hyphenpenaltyplus);
	secretMenu->addAction(hyphenpenaltymoins);
	
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
		qDebug()<<"Oy Rb"<<origine.y()<<theRect.bottom();
		if(origine.y() > theRect.bottom())
			break;
		theString = spec[i];
		if(theString.isEmpty())
			continue;
		node = new Node ( 0 );
		doGraph();
		distCache.clear();
		doLines();
		distCache.clear();
		doDraw();
// 		qDebug()<<"N"<<node->count();
		delete node;
		lines.clear();
		breakList.clear();
		hyphenList.clear();
	}
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
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
	hyphenList.clear();
	for ( int a ( 0 ) ; a < theString.count() ; ++a )
	{
		if ( QChar ( theString[a].lChar ).category() == QChar::Separator_Space )
			breakList << a+1;
		if( theString[a].isBreak )
		{
			breakList << a+1;
			hyphenList << a+1;
		}
	}
	breakList << theString.count();
	qDebug() <<"BREAKS"<<breakList.count();
	qDebug() <<"HYPHENS"<<hyphenList.count();
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
	int maxIndex(indices.count() - 1);
	qDebug()<<"SC IC"<<theString.count()<<indices.count();
	bool hasHyph(false);
	GlyphList curHyph;
	
	for ( int lIdx ( 0 ); lIdx < maxIndex ; ++lIdx )
	{
		int start1( /*!lIdx ?*/ indices[lIdx] /*: indices[lIdx] + 1*/);
		int end1(indices[ lIdx + 1 ]);
		
		GlyphList inList(theString.mid(start1 , end1 - start1 /*+ 1 */) );
		
		if(QChar (inList.first().lChar).category() == QChar::Separator_Space)
		{
			while(QChar (inList.first().lChar).category() == QChar::Separator_Space)
				inList.takeFirst();
		}
		if(QChar (inList.last().lChar).category() == QChar::Separator_Space)
		{
			while(QChar (inList.last().lChar).category() == QChar::Separator_Space)
				inList.takeLast();
		}
		
		
		QString dStr;
		QString dBk;
		foreach(RenderedGlyph rg, inList)
		{
			dStr += QChar(rg.lChar);
			dBk += rg.isBreak ? "#" : "_";
		}
		qDebug()<< "S"<<dStr;
		qDebug()<< "H"<<dBk;
		
// 		qDebug()<<"S E Sib Eib"<<start<<end<<theString.at(start).isBreak<<theString.at(end).isBreak;
		
		GlyphList lg;
		/// * *
		if(!hasHyph && !inList.last().isBreak)
		{
			qDebug()<<"/// * *";
			lg = inList;
		}
		/// = *
		else if(hasHyph && !inList.last().isBreak)
		{
			qDebug()<<"/// = *";
			
			GlyphList hr(curHyph);
			hasHyph = false;
			
			for (int ih(0); ih < hr.count(); ++ih)
			{
				lg << hr[ih];
			}
			
			while( !inList.isEmpty() && QChar( inList.first() .lChar ).category() != QChar::Separator_Space )
			{
				inList.takeFirst();
			}
				
			for ( int i ( 0 ); i < inList.count() ;++i )
				lg <<  inList.at ( i );
		
		
		}
		/// * =
		else if(!hasHyph && inList.last().isBreak)
		{			
			qDebug()<<"/// * =";
			GlyphList hr(inList.last().hyphen.first);
			hasHyph = true;
			curHyph = inList.last().hyphen.second;
			
			do
			{
				inList.takeLast();
			}while(QChar( inList.last().lChar ).category() != QChar::Separator_Space && !inList.isEmpty());
		
			QString dgS;
			for ( int i ( 0 ); i < inList.count() ;++i )
				lg <<  inList.at ( i );
		
			for (int ih(0); ih < hr.count(); ++ih)
			{
				lg <<  hr[ih];
				dgS +="("+ QString(QChar(lg.last().lChar))+")";
			}
			QString dbh;for(int h(0);h<curHyph.count();++h){dbh+="["+ QString(QChar(curHyph[h].lChar)) +"]";}qDebug()<<dgS<<"="<<dbh;
		
		}
		/// = =
		else if(hasHyph && inList.last().isBreak)
		{
			
			qDebug()<<"/// = =";
			GlyphList hr(curHyph);	
			GlyphList hr2(inList.last().hyphen.first);
			
			hasHyph = true;
			curHyph = inList.last().hyphen.second;		
			
			for (int ih(0); ih < hr.count(); ++ih)
			{
				lg <<  hr[ih];
			}
			
			
			while( !inList.isEmpty() && QChar( inList.first() .lChar ).category() != QChar::Separator_Space )
			{
				inList.takeFirst();
			}
		
			do
			{
				inList.takeLast();
			}while(QChar( inList.last().lChar ).category() != QChar::Separator_Space && !inList.isEmpty());
		
// 			QString dgS;
			for ( int i ( 0 ); i < inList.count() ;++i )
				lg <<  inList.at ( i );
		
			for (int ih(0); ih < hr2.count(); ++ih)
			{
				lg <<  hr2[ih];
// 				dgS +="("+ QString(QChar(lg.last().lChar))+")";
			}
// 			qDebug()<<dgS;
		
		}
	
		lines << lg;
	}
	for(int lI(0); lI<lines.count(); ++lI)
	{
		GlyphList& lg(lines[lI]);
		if(QChar (lg.first().lChar).category() == QChar::Separator_Space)
		{
			while(QChar (lg.first().lChar).category() == QChar::Separator_Space  && !lg.isEmpty())
				lg.takeFirst();
		}
		if(QChar (lg.last().lChar).category() == QChar::Separator_Space)
		{
			while(QChar (lg.last().lChar).category() == QChar::Separator_Space  && !lg.isEmpty())
				lg.takeLast();
		}
			
		if(lI != lines.count() - 1) 
		{
			QList<int> wsIds;
			double diff ( refW - distance ( 0, lg.count(), lg ) );
			for ( int ri ( 0 ); ri < lg.count() ; ++ri )
			{
				if ( lg.at ( ri ).glyph &&  QChar ( lg.at ( ri ).lChar ).category() == QChar::Separator_Space )
				{
					wsIds << ri;
				}
			}
			double shareLost ( diff / qMax ( 1.0 , ( double ) wsIds.count() ) );
		// 		qDebug()<< "D N W"<<diff<<wsIds.count()<< shareLost;
			for ( int wi ( 0 ); wi < wsIds.count(); ++wi )
			{
				lg[ wsIds[wi] ].xadvance += shareLost;
			}
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

	for ( int lIdx ( 0 ); lIdx < lines.count() ; ++lIdx )
	{
		if ( pen.y() > pageBottom )
			break;
		GlyphList refGlyph(lines[lIdx]);

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

double FMLayout::distance ( int start, int end, const GlyphList& gl, bool power  )
{
	if(distCache.contains(start))
	{
		if(distCache[start].contains(end))
			return distCache[start][end];
	}
	GlyphList gList = gl;
	if(QChar (gList.first().lChar).category() == QChar::Separator_Space)
	{
		while(QChar (gList.first().lChar).category() == QChar::Separator_Space && start < end)
		{
			++start;
		}
	}
	if(QChar (gList.last().lChar).category() == QChar::Separator_Space)
	{
		while(QChar (gList.last().lChar).category() == QChar::Separator_Space && end > start)
		{
			--end;
		}
	}
	
// 	QString dStr;
// 	for(int di(start); di < end; ++di )
// 	{
// 		dStr += QChar(gl[di].lChar);
// 	}
// 	qDebug()<< "SD"<<dStr;
	
// 	qDebug()<<"DIS C S E"<<gList.count()<< start<< end;
	double ret ( 0.0 );
	if ( end <= start )
	{
// 		qDebug()<<"ERR_LOGIC! S E"<< start<< end;
		return ret;
	}
	int EXend(end -1);
	
	if(!gList.at(start).isBreak && !gList.at(EXend).isBreak)
	{
// 		qDebug()<<". ." ;
		for ( int i ( start ); i < end ;++i )
			ret += gList.at ( i ).xadvance /*+ gList.at ( i ).xoffset*/;
	}
	else if(gList.at(start).isBreak && !gList.at(EXend).isBreak)
	{
// 		qDebug()<<"- ." ;
		GlyphList hr(gList.at(start).hyphen.second);
		for (int ih(0); ih < hr.count(); ++ih)
		{
// 			qDebug()<<"ih"<<ih;
			ret += hr[ih].xadvance;
		}
		int bp(start);
// 		qDebug()<<"bp"<<bp;
		while (  bp < end ) 
		{
			if(QChar( gList.at(bp).lChar ).category() != QChar::Separator_Space )
				++bp ;
			else
				break;
		}
		
		for ( int i ( bp ); i < end ;++i )
		{
// 			qDebug()<<"i tS.xa"<<i<<gList.at ( i ).xadvance;
			ret += gList.at ( i ).xadvance ;
		}
		
		
	}
	else if(!gList.at(start).isBreak && gList.at(EXend).isBreak)
	{
// 		qDebug()<<". -" ;
		int bp(end);
		while ( QChar( gList.at(bp).lChar ).category() != QChar::Separator_Space &&  bp > start ) --bp ;
		--bp;
		
		for ( int i ( start ); i < bp ;++i )
			ret += gList.at ( i ).xadvance /*+ gList.at ( i ).xoffset*/;
		
		GlyphList hr(gList.at(EXend).hyphen.first);
		for (int ih(0); ih < hr.count(); ++ih)
		{
			ret += hr[ih].xadvance;
		}
		
	}
	else if(gList.at(start).isBreak && gList.at(EXend).isBreak)
	{
// 		qDebug()<<"- -" ;
		GlyphList hr(gList.at(start).hyphen.second);
		for (int ih(0); ih < hr.count(); ++ih)
		{
			ret += hr[ih].xadvance;
		}
		int bpS(start);
		while (  bpS < end ) 
		{
			if(QChar( gList.at(bpS).lChar ).category() != QChar::Separator_Space )
				++bpS ;
			else
				break;
		}
		
		int bpE(end);
		while ( QChar( gList.at(bpE).lChar ).category() != QChar::Separator_Space &&  bpE > start ) --bpE ;
		--bpE;
		
		for ( int i ( bpS); i < bpE ;++i )
			ret += gList.at ( i ).xadvance /*+ gList.at ( i ).xoffset*/;
		
		GlyphList hr2(gList.at(EXend).hyphen.first);
		for (int ih(0); ih < hr2.count(); ++ih)
		{
			ret += hr2[ih].xadvance;
		}
	}

// 	qDebug()<<"SID" ;
	if(power)
	{
		distCache[start][end] = ret*ret;
		return ret*ret;
	}
// 	distCache[start][end] = ret;
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
	if(((double)l * adjustedSampleInter) >  theRect.bottom()) // Out of scope!
	{
		return OUT_OF_RECT;
	}
	return theRect.width();
}


void FMLayout::slotSP() {
	FM_LAYOUT_NODE_SOON_F *= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
emit updateLayout();}
void FMLayout::slotSM() {
	FM_LAYOUT_NODE_SOON_F /= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();}
void FMLayout::slotFP() {
	FM_LAYOUT_NODE_FIT_F *= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();}
void FMLayout::slotFM() {
	FM_LAYOUT_NODE_FIT_F /= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();}
void FMLayout::slotLP() {
	FM_LAYOUT_NODE_LATE_F *= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();}
void FMLayout::slotLM() {
	FM_LAYOUT_NODE_LATE_F /= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();}
void FMLayout::slotEP() {
	FM_LAYOUT_NODE_END_F *= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();}
void FMLayout::slotEM() {
	FM_LAYOUT_NODE_END_F /= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();}

void FMLayout::slotHP()
{
	FM_LAYOUT_HYPHEN_PENALTY *= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();
}

void FMLayout::slotHM()
{
	FM_LAYOUT_HYPHEN_PENALTY /= 2.0;
	qDebug() <<"S F L E H"<<FM_LAYOUT_NODE_SOON_F<<FM_LAYOUT_NODE_FIT_F<<FM_LAYOUT_NODE_LATE_F<<FM_LAYOUT_NODE_END_F<<FM_LAYOUT_HYPHEN_PENALTY;
	emit updateLayout();
}
