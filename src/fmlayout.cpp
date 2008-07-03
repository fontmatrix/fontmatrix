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
#include "fmlayoptwidget.h"
#include "typotek.h"
#include "textprogression.h"

#include <QDialog>
#include <QGridLayout>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
// #include <QProgressBar>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QTime>
#include <QAction>
#include <QMenu>
// #include <QMutexLocker>
#include <QMutex>
// #include <QWaitCondition>

#define OUT_OF_RECT 99999999.0

int fm_layout_total_nod_dbg;
int fm_layout_total_skip_nod_dbg;
int fm_layout_total_leaves_dbg;

Node::Vector::Vector() :n ( 0 ), distance ( 0.0 )
{
// 	qDebug()<<"CV empty";
}

Node::Vector::Vector ( Node * N, double D ) :n ( N ),distance ( D )
{
// 	qDebug()<<"CV n d"<<n->index<<distance;
}

Node::Vector::~ Vector()
{
// 	qDebug()<<"RV"<<n<<distance;
// 	if(n)delete n;
}

Node::Node ( int i ) :index ( i )
{
	++fm_layout_total_nod_dbg;
}

bool Node::hasNode ( int idx )
{
	for ( int i ( 0 ); i < nodes.count(); ++i )
	{
		if ( nodes[i].n->index == idx )
			return true;
	}
	return false;
}

Node::~Node()
{
// 	qDebug()<<"R"<< index;
	for ( int i ( 0 ); i < nodes.count(); ++i )
	{
		if ( nodes[i].n ) delete nodes[i].n;
	}
}

int Node::count()
{
	int c ( nodes.count() );
	for ( int i ( 0 ); i < c ; ++i )
	{
		c += nodes[i].n->count();
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
	if ( lyt->stopIt )
	{
		theScore = 0;
	}
	nodes.clear();
	// cIdx is first glyph of the line
	int cIdx ( index );
	// bIndex is the break which sits on cIdx
	int bIndex ( lyt->breakList.indexOf ( cIdx ) );
	bool wantPlus ( true );
	while ( wantPlus )
	{
		++bIndex;
		if ( bIndex  < lyt->breakList.count() )
		{
			// additive width of glyphs
			double di ( lyt->distance ( cIdx, lyt->breakList[bIndex],lyt->theString ) );
			if ( di >= lyt->lineWidth ( deep ) )
			{
				/// BEFORE
				for ( int backIndex ( 1 ) ; ( bIndex - backIndex > 0 ) && ( lyt->breakList[bIndex - backIndex] != cIdx ); ++backIndex )
				{
					int soon ( lyt->breakList[bIndex - backIndex] );
					double needWidth(lyt->distance ( cIdx, soon ,lyt->theString ));
					double needWidthStripped(lyt->distance ( cIdx, soon ,lyt->theString , true ) );
					double spaceWidth(needWidth - needWidthStripped);
					
					double disN = needWidth - lyt->lineWidth ( deep ) ;
					
					double compressValue( disN * 100.0 / spaceWidth  );
// 					qDebug()<<"PRE spaceWidth("<<spaceWidth<<") disN("<<disN<<") compressValue("<<compressValue<<")";
					if(compressValue > lyt->FM_LAYOUT_MAX_COMPRESSION)
					{
						break;
					}
// 					else
// 						qDebug()<<"["<<cIdx<<","<<soon<<"]spaceWidth("<<spaceWidth<<") disN("<<disN<<") compressValue("<<compressValue<<")";
					
					
					Node* sN = 0;
					sN = new Node ( soon );
					if ( lyt->hyphenList.contains ( soon ) )
						disN *= lyt->FM_LAYOUT_HYPHEN_PENALTY;

					Node::Vector vN ( sN, qAbs ( disN * lyt->FM_LAYOUT_NODE_SOON_F ) );
					if ( nodes.isEmpty() )
					{
						nodes.insert ( 0, vN );
					}
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
					if ( QChar ( lyt->theString[soon].lChar ).category() == QChar::Separator_Space )
						break;
				}
				/// AT CLOSEST BREAK (after)
				{
					int fit ( lyt->breakList[bIndex] );
					
					double needWidth(lyt->distance ( cIdx, fit,lyt->theString ));
					double needWidthStripped(lyt->distance ( cIdx, fit ,lyt->theString , true ) );
					double spaceWidth(needWidth - needWidthStripped);
					
					double disF =  needWidth - lyt->lineWidth ( deep );
					
					double compressValue( disF * 100.0 / spaceWidth  );
					if(compressValue <= lyt->FM_LAYOUT_MAX_COMPRESSION)
					{
// 						qDebug()<<"["<<cIdx<<","<<fit<<"]("<< lyt->sepCount(cIdx, fit ,lyt->theString) <<") spaceWidth("<<spaceWidth<<") disF("<<disF<<") compressValue("<<compressValue<<")";

						Node* sF = 0;
						sF = new Node ( fit );
						if ( lyt->hyphenList.contains ( fit ) )
							disF *= lyt->FM_LAYOUT_HYPHEN_PENALTY;
		
						Node::Vector vF ( sF,qAbs ( disF * lyt->FM_LAYOUT_NODE_FIT_F ) );
		// 				curNode->nodes << vF;
						if ( nodes.isEmpty() )
						{
							nodes.insert ( 0,vF );
						}
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
					}
				}
				/// AFTER
				for ( int nextIndex ( 1 ); bIndex + nextIndex <  lyt->breakList.count() ; ++nextIndex )
				{
					int late ( lyt->breakList[bIndex + nextIndex] );
					double needWidth(lyt->distance ( cIdx, late ,lyt->theString ));
					double needWidthStripped(lyt->distance ( cIdx, late ,lyt->theString , true ) );
					double spaceWidth(needWidth - needWidthStripped);
					
					double disL = needWidth - lyt->lineWidth ( deep );
					
					double compressValue(  disL * 100.0 / spaceWidth  );
					if(compressValue > lyt->FM_LAYOUT_MAX_COMPRESSION)
					{
// 						qDebug()<<"break_ cV ="<<compressValue;
						break;
					}
// 					else
// 						qDebug()<<"["<<cIdx<<","<<late<<"]spaceWidth("<<spaceWidth<<") disL("<<disL<<") compressValue("<<compressValue<<")";
					
					Node* sL = 0;
					sL = new Node ( late );
					if ( lyt->hyphenList.contains ( late ) )
						disL *= lyt->FM_LAYOUT_HYPHEN_PENALTY;

					Node::Vector vL ( sL, qAbs ( disL * lyt->FM_LAYOUT_NODE_LATE_F ) );
					if ( nodes.isEmpty() )
					{
						nodes.insert ( 0, vL );
					}
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
					if ( late < lyt->theString.count() && QChar ( lyt->theString[late].lChar ).category() == QChar::Separator_Space )
						break;
				}

				wantPlus = false;

			}
			else if ( lyt->lineWidth ( deep ) == OUT_OF_RECT )
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
				{
					nodes.insert ( 0, vN );
				}
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

// 	double dCorrection ( ( double ) theList.count() / ( double ) curList.count() );
// 	dCorrection = 1.0;
// 	qDebug()<<"COR tl.c cl.c"<<dCorrection<<theList.count()<<curList.count();
	while ( !nodes.isEmpty() )
	{
// 		Vector v = nodes.first() ;
		double d1 ( dist + nodes[0].distance / curList.count() /** dCorrection*/ );
		double d2 ( theScore / qMax ( 1.0, ( double ) theList.count() ) );
		if ( d1  <  d2 )
		{
			nodes[0].n->sPath ( dist + nodes[0].distance, curList, theList, theScore );
		}
		else
			++fm_layout_total_skip_nod_dbg;
		nodes.removeFirst();
	}

	if ( isLeaf )
	{
		double mDist ( dist / deep );
		double mScore ( theScore / theList.count() );
		qDebug() <<"D S N"<< mDist << mScore << curList;
		if ( mDist < mScore )
		{
			theScore = dist;
			theList = curList;
		}
		++fm_layout_total_leaves_dbg;
	}
}

FMLayout *FMLayout::instance = 0;
FMLayout::FMLayout ( /*QGraphicsScene * scene, FontItem * font */ )
{
	rules = new QGraphicsRectItem;
	instance = this;
	node = 0;
	layoutMutex = new QMutex;
	optionHasChanged = true;

// 	progressBar = new QProgressBar ;
// 	onSceneProgressBar = new QGraphicsProxyWidget ;
// 	onSceneProgressBar->setWidget ( progressBar );
// 	onSceneProgressBar->setZValue ( 1000 );

	connect ( this, SIGNAL ( paragraphFinished() ), this, SLOT( endOfParagraph() ) );
	connect ( this, SIGNAL ( layoutFinished() ), this, SLOT ( doDraw() ) );
	connect ( this, SIGNAL ( paintFinished() ), this, SLOT ( endOfRun() ) );

	FM_LAYOUT_NODE_SOON_F=	1200.0;
	FM_LAYOUT_NODE_FIT_F=	1000.0;
	FM_LAYOUT_NODE_LATE_F=	2000.0;
	FM_LAYOUT_NODE_END_F=	2500.0;
	FM_LAYOUT_HYPHEN_PENALTY = 1.5;
	FM_LAYOUT_MAX_COMPRESSION = 50.0; // 50%

	optionDialog = new QDialog ( typotek::getInstance() );
	optionDialog->setWindowTitle ( tr ( "Text engine options" ) );
	optionLayout =  new QGridLayout(optionDialog) ;
	
	optionsWidget = new FMLayOptWidget;
	optionsWidget->setRange ( FMLayOptWidget::BEFORE, 1, 10000 );
	optionsWidget->setRange ( FMLayOptWidget::EXACT, 1, 10000 );
	optionsWidget->setRange ( FMLayOptWidget::AFTER, 1, 10000 );
	optionsWidget->setRange ( FMLayOptWidget::END, 1, 10000 );
	optionsWidget->setRange ( FMLayOptWidget::HYPHEN, 1, 100 );
	optionsWidget->setRange ( FMLayOptWidget::SPACE, 1, 100 );

	optionsWidget->setValue ( FMLayOptWidget::BEFORE,FM_LAYOUT_NODE_SOON_F );
	optionsWidget->setValue ( FMLayOptWidget::EXACT,FM_LAYOUT_NODE_FIT_F );
	optionsWidget->setValue ( FMLayOptWidget::AFTER,FM_LAYOUT_NODE_LATE_F );
	optionsWidget->setValue ( FMLayOptWidget::END,FM_LAYOUT_NODE_END_F );
	optionsWidget->setValue ( FMLayOptWidget::HYPHEN,FM_LAYOUT_HYPHEN_PENALTY * 10 );
	optionsWidget->setValue ( FMLayOptWidget::SPACE,FM_LAYOUT_MAX_COMPRESSION );
	
	optionLayout->addWidget(optionsWidget,0,0);

	connect ( optionsWidget,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotOption ( int ) ) );

}

FMLayout::~ FMLayout()
{
}

void FMLayout::run()
{
	if(justRedraw)
		emit layoutFinished();
	else
	{
		for ( int i ( 0 ); i < paragraphs.count() ; ++ i )
		{
	// 		qDebug()<<"Oy Rb"<<origine.y()<<theRect.bottom();
			if ( origine.y() > theRect.bottom() )
				break;
			theString = paragraphs[i];
			if ( theString.isEmpty() )
				continue;
			node = new Node ( 0 );
			doGraph();
			clearCaches();
			{
				// Debug output
				fm_layout_total_leaves_dbg = 0;
				fm_layout_total_nod_dbg = 0;
				fm_layout_total_skip_nod_dbg = 0;
			}
	
			doLines();
	
			qDebug() <<"NODES LEAVES SKIP"<<fm_layout_total_nod_dbg<<fm_layout_total_leaves_dbg<<fm_layout_total_skip_nod_dbg;
			if ( node )
			{
				delete node;
				node = 0;
			}
			clearCaches();
			breakList.clear();
			hyphenList.clear();
// 			emit paragraphFinished ( i + 1 );
			emit paragraphFinished();
	
		}
		emit layoutFinished();
	}
}

void FMLayout::doLayout ( const QList<GlyphList> & spec , double fs )
{
	stopIt = false;

	TextProgression *tp = TextProgression::getInstance();

	if ( tp->inLine() == TextProgression::INLINE_LTR )
		origine.rx() = theRect.left() ;
	else if ( tp->inLine() == TextProgression::INLINE_RTL )
		origine.rx() = theRect.right() ;
	else if ( tp->inLine() == TextProgression::INLINE_BTT )
		origine.ry() = theRect.bottom();
	else if ( tp->inLine() == TextProgression::INLINE_TTB )
		origine.ry() = theRect.top();

	if ( tp->inBlock() == TextProgression::BLOCK_TTB )
		origine.ry() = theRect.top();
	else if ( tp->inBlock() == TextProgression::BLOCK_RTL )
		origine.rx() = theRect.right();
	else if ( tp->inBlock() == TextProgression::BLOCK_LTR )
		origine.rx() = theRect.left();

	qDebug()<<"LO"<<lastOrigine<<"O"<<origine<<"options"<<optionHasChanged;
	if( !optionHasChanged && origine == lastOrigine && fontSize == fs && paragraphs == spec )
	{
		justRedraw = true;
		typotek::getInstance()->startProgressJob( lines.count() );
		qDebug()<<"LAYOUT O : lines "<<lines.count();
	}
	else
	{
		qDebug()<<"LAYOUT 1";
		justRedraw = false;
		lines.clear();
		typotek::getInstance()->startProgressJob( paragraphs.count() + ( theRect.height() / fs*1.20 ) );// layout AND draw
	}
	lastOrigine = origine;
	fontSize = fs;
	paragraphs = spec;
	optionHasChanged = false;

	if ( node )
	{
		delete node;
		node = 0;
	}
}

void FMLayout::endOfRun()
{
	qDebug() <<"FMLayout::endOfRun()";
// 	theScene->removeItem ( onSceneProgressBar );
// 	disconnect ( this,SIGNAL ( paragraphFinished ( int ) ),progressBar,SLOT ( setValue ( int ) ) );
	if ( node )
	{
		delete node;
		node = 0;
	}
	qDebug()<<"EOR A"<<lines.count();
	layoutMutex->unlock();
	if ( stopIt ) // We’re here after a interruption
	{
		stopIt = false;
		typotek::getInstance()->endProgressJob();
		emit updateLayout();
	}
	else
		typotek::getInstance()->endProgressJob();
	
	qDebug()<<"EOR B"<<lines.count();
}

void FMLayout::stopLayout()
{
	stopIt = true;
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
		if ( theString[a].isBreak )
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
	
	clearCaches();
	// la messe est dite ! :-)
// 	qDebug() <<"S I"<<score<<indices;

	int startLine ( lines.count() );

	int maxIndex ( indices.count() - 1 );
// 	qDebug()<<"SC IC"<<theString.count()<<indices.count();
	bool hasHyph ( false );
	GlyphList curHyph;

	for ( int lIdx ( 0 ); lIdx < maxIndex ; ++lIdx )
	{
		if ( stopIt )
			break;
		int start1 ( /*!lIdx ?*/ indices[lIdx] /*: indices[lIdx] + 1*/ );
		int end1 ( indices[ lIdx + 1 ] );

		GlyphList inList ( theString.mid ( start1 , end1 - start1 /*+ 1 */ ) );

		if ( QChar ( inList.first().lChar ).category() == QChar::Separator_Space )
		{
			while ( QChar ( inList.first().lChar ).category() == QChar::Separator_Space )
				inList.takeFirst();
		}
		if ( QChar ( inList.last().lChar ).category() == QChar::Separator_Space )
		{
			while ( QChar ( inList.last().lChar ).category() == QChar::Separator_Space )
				inList.takeLast();
		}


		QString dStr;
		QString dBk;
		foreach ( RenderedGlyph rg, inList )
		{
			dStr += QChar ( rg.lChar );
			dBk += rg.isBreak ? "#" : "_";
		}
		qDebug() << "S"<<dStr;
		qDebug() << "H"<<dBk;

// 		qDebug()<<"S E Sib Eib"<<start<<end<<theString.at(start).isBreak<<theString.at(end).isBreak;

		GlyphList lg;
		/// * *
		if ( !hasHyph && !inList.last().isBreak )
		{
			qDebug() <<"/// * *";
			lg = inList;
		}
		/// = *
		else if ( hasHyph && !inList.last().isBreak )
		{
			qDebug() <<"/// = *";

			GlyphList hr ( curHyph );
			hasHyph = false;

			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				lg << hr[ih];
			}

			while ( !inList.isEmpty() && QChar ( inList.first() .lChar ).category() != QChar::Separator_Space )
			{
				inList.takeFirst();
			}

			for ( int i ( 0 ); i < inList.count() ;++i )
				lg <<  inList.at ( i );


		}
		/// * =
		else if ( !hasHyph && inList.last().isBreak )
		{
			qDebug() <<"/// * =";
			GlyphList hr ( inList.last().hyphen.first );
			hasHyph = true;
			curHyph = inList.last().hyphen.second;

			do
			{
				inList.takeLast();
			}
			while ( QChar ( inList.last().lChar ).category() != QChar::Separator_Space && !inList.isEmpty() );


			for ( int i ( 0 ); i < inList.count() ;++i )
				lg <<  inList.at ( i );

			QString dgS;
			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				lg <<  hr[ih];
				dgS +="("+ QString ( QChar ( lg.last().lChar ) ) +")";
			}
			QString dbh;for ( int h ( 0 );h<curHyph.count();++h ) {dbh+="["+ QString ( QChar ( curHyph[h].lChar ) ) +"]";}
			qDebug() <<dgS<<"="<<dbh;

		}
		/// = =
		else if ( hasHyph && inList.last().isBreak )
		{

			qDebug() <<"/// = =";
			GlyphList hr ( curHyph );
			GlyphList hr2 ( inList.last().hyphen.first );

			hasHyph = true;
			curHyph = inList.last().hyphen.second;

			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				lg <<  hr[ih];
			}


			while ( !inList.isEmpty() && QChar ( inList.first() .lChar ).category() != QChar::Separator_Space )
			{
				inList.takeFirst();
			}

			do
			{
				inList.takeLast();
			}
			while ( QChar ( inList.last().lChar ).category() != QChar::Separator_Space && !inList.isEmpty() );

			QString dgS;
			for ( int i ( 0 ); i < inList.count() ;++i )
				lg <<  inList.at ( i );

			for ( int ih ( 0 ); ih < hr2.count(); ++ih )
			{
				lg <<  hr2[ih];
				dgS +="("+ QString ( QChar ( lg.last().lChar ) ) +")";
			}
			QString dbh;for ( int h ( 0 );h<curHyph.count();++h ) {dbh+="["+ QString ( QChar ( curHyph[h].lChar ) ) +"]";}
			qDebug() <<dgS<<"="<<dbh;

		}

		lines << lg;
	}

	TextProgression *tp = TextProgression::getInstance();
	bool verticalLayout ( tp->inBlock() == TextProgression::INLINE_BTT || tp->inBlock() == TextProgression::INLINE_TTB );
	

	for ( int lI ( startLine ); lI<lines.count(); ++lI )
	{
		if ( stopIt )
			break;

		GlyphList& lg ( lines[lI] );
		
		if ( QChar ( lg.first().lChar ).category() == QChar::Separator_Space )
		{
			while ( QChar ( lg.first().lChar ).category() == QChar::Separator_Space  && !lg.isEmpty() )
				lg.takeFirst();
		}
		if ( QChar ( lg.last().lChar ).category() == QChar::Separator_Space )
		{
			while ( QChar ( lg.last().lChar ).category() == QChar::Separator_Space  && !lg.isEmpty() )
				lg.takeLast();
		}
		
		clearCaches();
		double refW ( lineWidth( lI ) );
		double actualW( distance ( 0, lg.count(), lg ) );
		double diff ( refW - actualW );
		if(!deviceIndy)
		{
// 			qDebug()<< "R1 A1"<<refW<<actualW;
			refW =  refW * 72.0  / ( double ) QApplication::desktop()->physicalDpiX() ;
			actualW = actualW * 72.0 / ( double ) QApplication::desktop()->physicalDpiX();
// 			qDebug()<< "R2 A2"<<refW<<actualW;
			diff = refW - actualW ;
			
		}

		if ( lI != lines.count() - 1 || actualW > refW ) // not last line or last line is too long
		{
			QList<int> wsIds;
// 			qDebug()<< "Ref Dis"<< refW << distance ( 0, lg.count(), lg );
			for ( int ri ( 0 ); ri < lg.count() ; ++ri )
			{
				if ( lg.at ( ri ).glyph &&  QChar ( lg.at ( ri ).lChar ).category() == QChar::Separator_Space )
				{
					wsIds << ri;
				}
			}
			double shareLost ( diff / qMax ( 1.0 , ( double ) wsIds.count() ) );
//  			if(!oldIndy)
// 				shareLost *= ( double ) QApplication::desktop()->physicalDpiX() / 72.0;
// 			qDebug() << "D N W"<<diff<<wsIds.count() << shareLost ;
// 			qDebug()<<"R D F"<< refW << actualW << actualW + ((double)wsIds.count() * shareLost);
			if ( verticalLayout )
			{
				for ( int wi ( 0 ); wi < wsIds.count(); ++wi )
				{
					lg[ wsIds[wi] ].yadvance += shareLost;
				}
			}
			else
			{
				for ( int wi ( 0 ); wi < wsIds.count(); ++wi )
				{
					lg[ wsIds[wi] ].xadvance += shareLost;
				}
			}
		}
	}
// 	qDebug() <<"doLines("<< lines.count() <<") T(ms)"<<t.elapsed();

}

void FMLayout::doDraw()
{
	// Ask paths or pixmaps to theFont for each glyph and draw it on theScene
	qDebug() <<"FMLayout::doDraw()";
	resetScene();
	QTime t;
	t.start();
	TextProgression *tp = TextProgression::getInstance();
	QPointF pen ( origine );
	if( tp->inLine() != TextProgression::INLINE_BTT )
		pen.ry() += adjustedSampleInter;
	if( tp->inBlock() == TextProgression::BLOCK_RTL )
		pen.rx() -= adjustedSampleInter;
	
	double pageTop(theRect.top());
	double pageRight(theRect.right());
	double pageBottom ( theRect.bottom() );
	double pageLeft(theRect.left());
	
	double scale = fontSize / theFont->getUnitPerEm();
	double pixelAdjustX = ( double ) QApplication::desktop()->physicalDpiX() / 72.0 ;
	double pixelAdjustY = ( double ) QApplication::desktop()->physicalDpiX() / 72.0 ;


	for ( int lIdx ( 0 ); lIdx < lines.count() ; ++lIdx )
	{
		if ( stopIt )
			break;
		if ( tp->inLine() != TextProgression::INLINE_BTT &&  pen.y() > pageBottom )
			break;
		else if(tp->inLine() == TextProgression::INLINE_BTT || tp->inLine() == TextProgression::INLINE_TTB)
		{
			if(tp->inBlock() == TextProgression::BLOCK_RTL && pen.x() < pageLeft)
				break;
			else if(tp->inBlock() == TextProgression::BLOCK_LTR && pen.x() > pageRight)
				break;
		}
		clearCaches();
		GlyphList refGlyph ( lines[lIdx] );

		if ( !deviceIndy )
		{
			for ( int i=0; i < refGlyph.count(); ++i )
			{
				if ( !refGlyph[i].glyph )
					continue;
				QGraphicsPixmapItem *glyph = theFont->itemFromGindexPix ( refGlyph[i].glyph , fontSize );
				if ( !glyph )
					continue;
				if ( tp->inLine() == TextProgression::INLINE_RTL )
				{
					pen.rx() -= refGlyph[i].xadvance * pixelAdjustX ;
				}
				else if ( tp->inLine() == TextProgression::INLINE_BTT )
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

				if ( tp->inLine() == TextProgression::INLINE_LTR )
					pen.rx() += refGlyph[i].xadvance * pixelAdjustX ;
				else if ( tp->inLine() == TextProgression::INLINE_TTB )
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

				if ( tp->inLine() == TextProgression::INLINE_RTL )
				{
					pen.rx() -= refGlyph[i].xadvance ;
				}
				else if ( tp->inLine() == TextProgression::INLINE_BTT )
				{
					pen.ry() -= refGlyph[i].yadvance ;
				}
				/**********************************************/

				theScene->addItem ( glyph );
				glyph->setPos ( pen.x() + ( refGlyph[i].xoffset ),
				                pen.y() + ( refGlyph[i].yoffset ) );
				glyph->setZValue ( 100.0 );
				/*******************************************/

				if ( tp->inLine() == TextProgression::INLINE_LTR )
					pen.rx() += refGlyph[i].xadvance ;
				else if ( tp->inLine() == TextProgression::INLINE_TTB )
					pen.ry() += refGlyph[i].yadvance;

			}
		}

		if ( tp->inBlock() == TextProgression::BLOCK_TTB )
		{
			pen.ry() += adjustedSampleInter;
			pen.rx() = origine.x() ;
		}
		else if ( tp->inBlock() == TextProgression::BLOCK_RTL )
		{
			if(tp->inLine() == TextProgression::INLINE_TTB)
				pen.ry() = origine.y() + adjustedSampleInter;
			else
				pen.ry() = origine.y();
			pen.rx() -= adjustedSampleInter;
		}
		else if ( tp->inBlock() == TextProgression::BLOCK_LTR )
		{
			if(tp->inLine() == TextProgression::INLINE_TTB)
				pen.ry() = origine.y() + adjustedSampleInter;
			else
				pen.ry() = origine.y();
			pen.rx() += adjustedSampleInter;
		}
		
		typotek::getInstance()->runProgressJob();
// 		qDebug() <<"P"<<pen;
	}

	qDebug() <<"doDraw T(ms)"<<t.elapsed();
	emit paintFinished();
	theScene->update(theRect);
}

int FMLayout::sepCount(int start, int end, const GlyphList & gl)
{
	if ( sepCache.contains ( start ) )
	{
		if ( sepCache[start].contains ( end ) )
			return sepCache[start][end];
	}
	int storeStart(start);
	int storeEnd(end);
	
	GlyphList gList = gl;
	if ( QChar ( gList.first().lChar ).category() == QChar::Separator_Space )
	{
		while ( QChar ( gList.first().lChar ).category() == QChar::Separator_Space && start < end )
		{
			++start;
		}
	}
	if ( QChar ( gList.last().lChar ).category() == QChar::Separator_Space )
	{
		while ( QChar ( gList.last().lChar ).category() == QChar::Separator_Space && end > start )
		{
			--end;
		}
	}
	int ret(0);
	for ( int i ( start ); i < end ;++i )
	{
		if(QChar ( gList.at( i ).lChar  ).category() == QChar::Separator_Space )
			++ret;
	}
	sepCache[storeStart][storeEnd] = ret;
	return ret;
}

// please move this method to GlyphList itself
double FMLayout::distance ( int start, int end, const GlyphList& gl, bool strip )
{
// 	qDebug()<<"distance(start ="<<start<<",end"<<end<<",strip"<<strip<<" )";
	if(!strip)
	{
		if ( distCache.contains ( start ) )
		{
			if ( distCache[start].contains ( end ) )
				return distCache[start][end];
		}
	}
	else
	{
		if ( stripCache.contains ( start ) )
		{
			if ( stripCache[start].contains ( end ) )
				return stripCache[start][end];
		}
	}
	int storeStart(start);
	int storeEnd(end);
	bool hasHyph(false);
	GlyphList hyphList;
	if(start>0 && gl[start - 1].isBreak)
	{
		hasHyph = true;
		hyphList = gl[start - 1].hyphen.second;
	}
	GlyphList gList = gl;
// 	if ( QChar ( gList.first().lChar ).category() == QChar::Separator_Space )
// 	{
// 		while ( QChar ( gList.first().lChar ).category() == QChar::Separator_Space && start < end )
// 		{
// 			++start;
// 		}
// 	}
// 	if ( QChar ( gList.last().lChar ).category() == QChar::Separator_Space )
// 	{
// 		while ( QChar ( gList.last().lChar ).category() == QChar::Separator_Space && end > start )
// 		{
// 			--end;
// 		}
// 	}
// 	
	TextProgression *tp = TextProgression::getInstance();
	bool verticalLayout ( tp->inLine() == TextProgression::INLINE_BTT || tp->inLine() == TextProgression::INLINE_TTB );
// 	qDebug()<<"IB VL"<<tp->inLine()<<verticalLayout;
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
	int EXend ( end -1 );
	if ( verticalLayout )
	{
		if ( !hasHyph && !gList.at ( EXend ).isBreak )
		{
			for ( int i ( start ); i < end ;++i )
			{
// 				qDebug()<<"Ya"<< gList.at ( i ).yadvance;
				if(strip)
				{
					if( QChar ( gList.last().lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).yadvance;
				}
				else
					ret += gList.at ( i ).yadvance;
			}
		}
		else if ( hasHyph && !gList.at ( EXend ).isBreak )
		{
			GlyphList hr ( gList.at ( start - 1).hyphen.second );
			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				ret += hr[ih].yadvance;
			}
			int bp ( start );
			while ( bp < end )
			{
				if ( QChar ( gList.at ( bp ).lChar ).category() != QChar::Separator_Space )
					++bp ;
				else
					break;
			}

			for ( int i ( bp ); i < end ;++i )
			{
				if(strip)
				{
					if( QChar ( gList.last().lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).yadvance;
				}
				else
					ret += gList.at ( i ).yadvance ;
			}
		}
		else if ( !hasHyph && gList.at ( EXend ).isBreak )
		{
			int bp ( end );
			while ( QChar ( gList.at ( bp ).lChar ).category() != QChar::Separator_Space &&  bp > start ) --bp ;
			++bp;

			for ( int i ( start ); i < bp ;++i )
			{
				if(strip)
				{
					if( QChar ( gList.last().lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).yadvance;
				}
				else
					ret += gList.at ( i ).yadvance;
			}
			GlyphList hr ( gList.at ( EXend ).hyphen.first );
			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				ret += hr[ih].yadvance;
			}

		}
		else if ( hasHyph && gList.at ( EXend ).isBreak )
		{
			GlyphList hr ( hyphList );
			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				ret += hr[ih].yadvance;
			}
			int bpS ( start );
			while ( bpS < end )
			{
				if ( QChar ( gList.at ( bpS ).lChar ).category() != QChar::Separator_Space )
					++bpS ;
				else
					break;
			}

			int bpE ( end );
			while ( QChar ( gList.at ( bpE ).lChar ).category() != QChar::Separator_Space &&  bpE > start ) --bpE ;
			++bpE;

			for ( int i ( bpS ); i < bpE ;++i )
			{
				if(strip)
				{
					if( QChar ( gList.last().lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).yadvance;
				}
				else
					ret += gList.at ( i ).yadvance;
			}
			GlyphList hr2 ( gList.at ( EXend ).hyphen.first );
			for ( int ih ( 0 ); ih < hr2.count(); ++ih )
			{
				ret += hr2[ih].yadvance;
			}
		}
	}
	else
	{
		if ( !hasHyph && !gList.at ( EXend ).isBreak )
		{
// 					qDebug()<<". ." ;
			for ( int i ( start ); i < end ;++i )
			{
				if(strip)
				{
					if( QChar ( gList.at(i).lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).xadvance;
				}
				else
					ret += gList.at ( i ).xadvance /*+ gList.at ( i ).xoffset*/;
			}
		}
		else if ( hasHyph && !gList.at ( EXend ).isBreak )
		{
// 					qDebug()<<"- ." ;
			GlyphList hr ( hyphList );
			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				// 			qDebug()<<"ih"<<ih;
				ret += hr[ih].xadvance;
			}
			int bp ( start );
			// 		qDebug()<<"bp"<<bp;
			while ( bp < end )
			{
				if ( QChar ( gList.at ( bp ).lChar ).category() != QChar::Separator_Space )
					++bp ;
				else
					break;
			}

			for ( int i ( bp ); i < end ;++i )
			{
				// 			qDebug()<<"i tS.xa"<<i<<gList.at ( i ).xadvance;
				if(strip)
				{
					if( QChar ( gList.at(i).lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).xadvance;
				}
				else
					ret += gList.at ( i ).xadvance ;
			}


		}
		else if ( !hasHyph && gList.at ( EXend ).isBreak )
		{
// 					qDebug()<<". -" ;
			int bp ( end );
			while ( QChar ( gList.at ( bp ).lChar ).category() != QChar::Separator_Space &&  bp > start ) --bp ;
			++bp;

			for ( int i ( start ); i < bp ;++i )
			{
				if(strip)
				{
					if( QChar ( gList.at(i).lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).xadvance;
				}
				else
					ret += gList.at ( i ).xadvance /*+ gList.at ( i ).xoffset*/;
			}
			GlyphList hr ( gList.at ( EXend ).hyphen.first );
			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
				ret += hr[ih].xadvance;
			}

		}
		else if ( hasHyph && gList.at ( EXend ).isBreak )
		{
// 					qDebug()<<"- -" ;
// 			QString debStr;
			GlyphList hr ( hyphList);
			for ( int ih ( 0 ); ih < hr.count(); ++ih )
			{
// 				debStr += QChar(hr[ih].lChar);
				ret += hr[ih].xadvance;
			}
			
			int bpS ( start );
			while ( bpS < end )
			{
				if ( QChar ( gList.at ( bpS ).lChar ).category() != QChar::Separator_Space )
					++bpS ;
				else
					break;
			}

			int bpE ( end );
			while ( QChar ( gList.at ( bpE ).lChar ).category() != QChar::Separator_Space &&  bpE > start ) --bpE ;
			++bpE;

			for ( int i ( bpS ); i < bpE ;++i )
			{
				if(strip)
				{
					if( QChar ( gList.at(i).lChar ).category() != QChar::Separator_Space)
						ret += gList.at ( i ).xadvance;
				}
				else
				{
// 					debStr += QChar(gList.at( i ).lChar);
					ret += gList.at ( i ).xadvance /*+ gList.at ( i ).xoffset*/;
				}
			}
			GlyphList hr2 ( gList.at ( EXend ).hyphen.first );
			for ( int ih ( 0 ); ih < hr2.count(); ++ih )
			{
// 				debStr += QChar(hr2[ih].lChar);
				ret += hr2[ih].xadvance;
			}
// 			qDebug()<<"D(= =)"<<debStr;
		}
	}

// 	qDebug()<<"SID" ;
	if(!deviceIndy)
		ret *= ( double ) QApplication::desktop()->physicalDpiX() / 72.0 ;
// 	if ( power )
// 	{
// 		distCache[storeStart][storeEnd] = ret*ret;
// 		return ret*ret;
// 	}
	distCache[start][end] = ret;
	return ret;
}

void FMLayout::resetScene()
{
// 	qDebug() <<"FMLayout::resetScene(P"<<pixList.count() <<",G"<<glyphList.count() <<")";
// 	QTime t;
// 	t.start();
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
// 	qDebug() <<"resetScene T(ms)"<<t.elapsed();
}

void FMLayout::setTheScene ( QGraphicsScene* theValue )
{
	theScene = theValue;
	QRectF tmpRect = theScene->sceneRect();
	double sUnitW ( tmpRect.width() * .1 );
	double sUnitH ( tmpRect.height() * .1 );

	theRect.setX ( 2.0 * sUnitW );
	theRect.setY ( 1.0 * sUnitH );
	theRect.setWidth ( 6.0 * sUnitW );
	theRect.setHeight ( 8.0 * sUnitH );
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
	TextProgression *tp = TextProgression::getInstance();
	double offset ( ( double ) l * adjustedSampleInter ) ;
	if ( tp->inBlock() == TextProgression::BLOCK_TTB )
	{
// 		if ( theRect.top() + offset >  theRect.bottom() ) 
// 		{
// 			return OUT_OF_RECT;
// 		}
		return theRect.width();
	}
	else if ( tp->inBlock() == TextProgression::BLOCK_RTL )
	{
// 		if ( theRect.right() - offset <  theRect.left() ) 
// 		{
// 			return OUT_OF_RECT;
// 		}
		return theRect.height() - adjustedSampleInter;
	}
	else if ( tp->inBlock() == TextProgression::BLOCK_LTR )
	{
// 		if ( theRect.left() + offset >  theRect.right() ) 
// 		{
// 			return OUT_OF_RECT;
// 		}
		return theRect.height() - adjustedSampleInter;
	}

	return OUT_OF_RECT;
}

void FMLayout::slotOption ( int v )
{
	optionHasChanged = true;
	
	if ( v == FMLayOptWidget::BEFORE )
	{
		FM_LAYOUT_NODE_SOON_F = optionsWidget->getValue ( FMLayOptWidget::BEFORE );
		emit updateLayout();
	}
	else if ( v == FMLayOptWidget::EXACT )
	{
		FM_LAYOUT_NODE_FIT_F = optionsWidget->getValue ( FMLayOptWidget::EXACT );
		emit updateLayout();
	}
	else if ( v == FMLayOptWidget::AFTER )
	{
		FM_LAYOUT_NODE_LATE_F = optionsWidget->getValue ( FMLayOptWidget::AFTER );
		emit updateLayout();
	}
	else if ( v == FMLayOptWidget::END )
	{
		FM_LAYOUT_NODE_END_F = optionsWidget->getValue ( FMLayOptWidget::END );
		emit updateLayout();
	}
	else if ( v == FMLayOptWidget::HYPHEN )
	{
		FM_LAYOUT_HYPHEN_PENALTY = ( double ) optionsWidget->getValue ( FMLayOptWidget::HYPHEN ) / 10.0;
		emit updateLayout();
	}
	else if ( v == FMLayOptWidget::SPACE )
	{
		FM_LAYOUT_MAX_COMPRESSION = optionsWidget->getValue ( FMLayOptWidget::SPACE );
		emit updateLayout();
	}
}

void FMLayout::setAdjustedSampleInter(double theValue)
{
	adjustedSampleInter = !deviceIndy ? theValue * ( double ) QApplication::desktop()->physicalDpiX() / 72.0 : theValue;
}

void FMLayout::clearCaches()
{
	sepCache.clear();
	distCache.clear();
	stripCache.clear();
}

void FMLayout::endOfParagraph()
{
	typotek::getInstance()->runProgressJob();
}




