//
// C++ Implementation: parallelcoor
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "parallelcoor.h"
#include "typotek.h"


#include <QGraphicsScene>
#ifdef HAVE_QTOPENGL
#include <QGLWidget>
#endif
#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItemGroup>

/**
	DataSet
*/
const QString ParallelCoorDataSet::FieldSep = ":";

ParallelCoorDataSet::ParallelCoorDataSet()
	:QList< QPair<QString, QList<QString> > >()
{
}

ParallelCoorDataSet::~ ParallelCoorDataSet()
{
}


QMap< QString , QString> ParallelCoorDataSet::getCategoryDescriptions() const
{
	return m_categoryDescriptions;
}


void ParallelCoorDataSet::setCategoryDescriptions ( const QMap< QString , QString >& theValue )
{
	m_categoryDescriptions = theValue;
}


QMap< QString , QString> ParallelCoorDataSet::getValueDescriptions() const
{
	return m_valueDescriptions;
}


void ParallelCoorDataSet::setValueDescriptions ( const QMap< QString , QString >& theValue )
{
	m_valueDescriptions = theValue;
}

ParallelCoorDataType ParallelCoorDataSet::getData() const
{
	return m_data;
}


void ParallelCoorDataSet::setData ( const ParallelCoorDataType& theValue )
{
	m_data = theValue;
}


/**
	View
*/
ParallelCoorView::ParallelCoorView(QWidget * parent)
	:QGraphicsView(parent), m_dataSet(0)
{
	setScene(new QGraphicsScene(this));
#ifdef HAVE_QTOPENGL
	QGLFormat glfmt;
	glfmt.setSampleBuffers ( true );
	QGLWidget *glwgt = new QGLWidget ( glfmt );
	if ( glwgt->format().sampleBuffers() )
	{
		setViewport ( glwgt );
		setRenderHint(QPainter::Antialiasing,true);
	}
	else
	{
		delete glwgt;
		setRenderHint(QPainter::Antialiasing,false);
	}
#endif
}

ParallelCoorView::ParallelCoorView(ParallelCoorDataSet * dataset, QWidget * parent)
	:QGraphicsView(parent), m_dataSet(dataset)
{
	setScene(new QGraphicsScene(this));
#ifdef HAVE_QTOPENGL
	QGLFormat glfmt;
	glfmt.setSampleBuffers ( true );
	QGLWidget *glwgt = new QGLWidget ( glfmt );
	if ( glwgt->format().sampleBuffers() )
	{
		setViewport ( glwgt );
		setRenderHint(QPainter::Antialiasing,true);
	}
	else
	{
		delete glwgt;
		setRenderHint(QPainter::Antialiasing,false);
	}
#endif
}

ParallelCoorView::~ParallelCoorView()
{
}

void ParallelCoorView::selectField(const QString & field)
{
	emit selectedField(field);
	setCurrentField(field);
}

QString ParallelCoorView::getCurrentField() const
{
	return m_currentField;
}

void ParallelCoorView::setCurrentField ( const QString& theValue )
{
// 	qDebug()<<"ParallelCoorView::setCurrentField"<<theValue<<m_currentField;
	if(theValue!=m_currentField)
	{
		m_currentField = theValue;
		redraw();
	}
}

ParallelCoorDataSet* ParallelCoorView::getDataSet() const
{
	return m_dataSet;
}


void ParallelCoorView::setDataSet ( ParallelCoorDataSet* theValue )
{
	m_dataSet = theValue;
}

ParallelCoorView::Units::Units(int width, int height, int count)
{			
	hunit = static_cast<double> ( height ) /1000.0 ;
	wunit = static_cast<double> ( width ) /1000.0  ;
	XOffset = hunit * 200.0;
	YOffset = hunit * 100.0 ;
	H = hunit * 800.0 ;
	W = wunit * 700.0 ;
	C = count ;
	step = W / static_cast<double> ( C-1 ) ;
}

void ParallelCoorView::cleanLists()
{
// 	qDebug()<<"ParallelCoorView::cleanLists";
	
// 	scene()->removeItem(scene()->createItemGroup(scene()->items()));
// 	qDebug()<<"D valueLabels"<<valueLabels.count();
	foreach(ParallelCoorValueItem* ti, valueLabels)
	{
// 		scene()->removeItem(ti);
		delete ti;
	}
// 	qDebug()<<"D fieldLabels"<<fieldLabels.count();
	foreach(ParallelCoorFieldItem* ti, fieldLabels)
	{
// 		scene()->removeItem(ti);
		delete ti;
	}
// 	qDebug()<<"D vertices"<<vertices.count();
	foreach(QGraphicsLineItem* pi, vertices)
	{
// 		scene()->removeItem(pi);
		delete pi;
	}
// 	qDebug()<<"D bars"<<bars.count();
	foreach(ParallelCoorBarItem* li, bars)
	{
// 		scene()->removeItem(li);
		delete li;
	}
	
// 	qDebug()<<"Clearing lists";
	valueLabels.clear();
	fieldLabels.clear();
	vertices.clear();
	bars.clear();
}

void ParallelCoorView::redraw()
{
// 	qDebug()<<"ParallelCoorView::redraw";
	cleanLists();

	if ( !m_dataSet )
		return;
	if ( m_dataSet->isEmpty() )
		return;

	units = Units(width(), height(), m_dataSet->count());
	drawBars();
	drawVertices();
	drawFields();
	drawValues();
}

void ParallelCoorView::drawBars()
{
// 	qDebug()<<"ParallelCoorView::drawBars";
	QPen pen ( Qt::gray , 3.0 );
	for ( int i ( 0 ); i < units.C ; ++i )
	{
		double di ( static_cast<double> ( i ) );
		QLineF bl(units.XOffset + ( di * units.step ),  units.YOffset,
			  units.XOffset + ( di * units.step ),  units.YOffset + units.H);
		ParallelCoorBarItem * bi ( new ParallelCoorBarItem(m_dataSet->at(i).first, this) );
		bars << bi;
		bi->setPen(pen);
		bi->setLine(bl);
		scene()->addItem(bi);
	}

}

void ParallelCoorView::drawVertices()
{
	const int N ( m_dataSet->getData().count() );
// 	qDebug() <<"ParallelCoorView::drawVertices"<<N * 9;
	QMap<int, QMap< int, QPointF> > placeCoords;
	for ( int k ( 0 );k < m_dataSet->count(); ++k )
	{
		QStringList list ( m_dataSet->at ( k ).second );
		double x ( units.XOffset + ( static_cast<double> ( k ) * units.step ) );
		double v ( units.H / static_cast<double> ( list.count()-1 ) );
		for ( int l ( 0 ); l < list.count(); ++l )
		{
			double y ( units.YOffset + ( static_cast<double> ( l ) * v ) );
			placeCoords[k][l] = QPointF ( x,y );
		}
	}
	QPen penFilter ( Qt::black, 1.0 );
	QPen penUnFilter ( Qt::lightGray, 0.5 );
	
	QList<QLineF> cflines;
	QList<QLineF> culines;
	
	for ( int a ( 0 );a<N;++a )
	{
// 		if( m_dataSet->getData().at(a).count() == m_dataSet->count() )
		{
			QList<QPointF> pol;
			for ( int b ( 0 ); b < m_dataSet->getData().at ( a ).count() ; ++b )
			{
				if(placeCoords[b].contains( m_dataSet->getData().at ( a ).at ( b ) ))
					pol << QPointF( placeCoords[b][m_dataSet->getData().at ( a ).at ( b ) ] );
				else
					pol << QPointF( placeCoords[b][0] );

			}
			if ( pol.count() == m_dataSet->count() )
			{
				for(int vi(1);vi<pol.count();++vi)
				{
					bool f ( matchFilter ( m_dataSet->getData().at ( a ) ) );
					QLineF lf(pol[vi-1],pol[vi]);
					if(f ? (!cflines.contains(lf)) : (!culines.contains(lf)))
					{
						vertices << scene()->addLine ( lf, f ? penFilter : penUnFilter );
						vertices.last()->setZValue(f ? 100.0 : 1.0);
						f ? (cflines << lf) : (culines << lf) ;
					}
// 					else
// 						qDebug()<<"Cached Line"<<(f?cflines.indexOf(lf):culines.indexOf(lf)) ;
				}
			}
		}
	}
// 	qDebug()<<"R"<< cflines.count() << culines.count();
}

void ParallelCoorView::drawFields()
{
// 	qDebug()<<"ParallelCoorView::drawFields";
	QFont fontF( "Helvetica" , 9, QFont::DemiBold, false );
	for(int k(0);k < m_dataSet->count(); ++k)
	{
		QString f(m_dataSet->at(k).first);
		ParallelCoorFieldItem * ti = new ParallelCoorFieldItem(f,this);
		ti->setFont(fontF);
		fieldLabels << ti;
		
		scene()->addItem(ti);
// 		double w(ti->boundingRect().width());
		ti->setPos(units.XOffset + (k*units.step) /*- (w/3.0)*/, units.H+units.YOffset);
	}
	
}

void ParallelCoorView::drawValues()
{
// 	qDebug()<<"ParallelCoorView::drawValues";
	int di(0);
	if(!m_currentField.isEmpty())
	{
		for(int i(0);i<m_dataSet->count();++i)
		{
			if(m_dataSet->at(i).first == m_currentField)
			{
				di = i;
				break;
			}
		}
	}
	else
		m_currentField = m_dataSet->at(0).first;
	
	QFont fontV( "Helvetica" , 9, QFont::DemiBold , true );
	double din(static_cast<double>(di));
	double dn(static_cast<double>(m_dataSet->at(di).second.count()-1));
	double vsep(units.H / dn);
	QList<QString> list (m_dataSet->at(di).second);
	for(int i(0); i< list.count(); ++i)
	{
		ParallelCoorValueItem *vi = new ParallelCoorValueItem(list[i], this);
		valueLabels << vi;
		vi->setFont(fontV);
		scene()->addItem(vi);
		
		double w((units.XOffset * .9) - vi->boundingRect().width());
		double h(vi->boundingRect().height() / 2.0);
// 		qDebug()<<"V"<<w<<vi->boundingRect().width()<<units.XOffset;
		vi->setPos( w , units.YOffset + (static_cast<double>(i) * vsep) - h);
		vi->setZValue(1000.0);
	}
}

void ParallelCoorView::updateGraphic()
{
	redraw();
}

void ParallelCoorView::resizeEvent(QResizeEvent * event)
{
	redraw();
	QGraphicsView::resizeEvent(event);
}


QMap< QString, QStringList > ParallelCoorView::getFilter() const
{
	return m_filter;
}


void ParallelCoorView::setFilter ( const QMap< QString, QStringList >& theValue )
{
	m_filter.clear();
	cfilter.clear();
	
	m_filter = theValue;
	
	for(int i(0);i < m_dataSet->count(); i++)
	{
		if(m_filter.contains(m_dataSet->at(i).first))
		{
			foreach(QString v, m_filter[m_dataSet->at(i).first])
			{
				cfilter[i] << m_dataSet->at(i).second.indexOf(v);
			}
		}
	}
	
	emit filterChanged();
	redraw();
}



bool ParallelCoorView::matchFilter(QList< int > list) const
{
	if(list.isEmpty())
	{
// 		qDebug()<<"List empty";
		return false;
	}
	if(m_filter.isEmpty())
	{
// 		qDebug()<<"Filter empty";
		return true;
	}
	
	bool ret(false);
	for(int i(0); i < list.count();++i)
	{
		if(cfilter.contains(i))
		{
			if(cfilter[i].contains(list[i]))
				ret = true;
			else
			{
				ret = false;
				break;
			}
		}
	}
	return ret;
}


/**
	Field label
*/
void ParallelCoorFieldItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
	qApp->setOverrideCursor(Qt::PointingHandCursor);
	QBrush b = brush();
	b.setColor(Qt::red);
	setBrush(b);
}

void ParallelCoorFieldItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
	QBrush b = brush();
	b.setColor(Qt::black);
	setBrush(b);
	qApp->restoreOverrideCursor();
}

void ParallelCoorFieldItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	event->accept();
}

void ParallelCoorFieldItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if(QString(pview->metaObject()->className()) == QString("ParallelCoorView") )
	{
		ParallelCoorView *pcv = reinterpret_cast<ParallelCoorView*>(pview);
		pcv->selectField(text());
	}
	
	qApp->restoreOverrideCursor();
}

ParallelCoorFieldItem::ParallelCoorFieldItem(QString text, QGraphicsView* pcv, QGraphicsItem * parent)
	:QGraphicsSimpleTextItem(parent), pview(pcv)
{
	setText(text);
	setEnabled(true);
	setAcceptHoverEvents ( true );
}

/**
	Value label
 */
ParallelCoorValueItem::ParallelCoorValueItem(QString text, QGraphicsView * pcv, QGraphicsItem * parent)
	:QGraphicsSimpleTextItem(parent), pview(pcv)
{
	setText(text);
	setEnabled(true);
	setAcceptHoverEvents ( true );
}

void ParallelCoorValueItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
	qApp->setOverrideCursor(Qt::PointingHandCursor);
	QBrush b = brush();
	b.setColor(Qt::red);
	setBrush(b);
}

void ParallelCoorValueItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
	QBrush b = brush();
	b.setColor(Qt::black);
	setBrush(b);
	qApp->restoreOverrideCursor();
}

void ParallelCoorValueItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	event->accept();
}

void ParallelCoorValueItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if(QString(pview->metaObject()->className()) == QString("ParallelCoorView") )
	{
		ParallelCoorView *pcv = reinterpret_cast<ParallelCoorView*>(pview);
		
		QMap<QString, QStringList> filter;
		if(event->modifiers() & Qt::ShiftModifier)
			filter = pcv->getFilter();
		filter[pcv->getCurrentField()] << text();
		pcv->setFilter(filter);
	}
	qApp->restoreOverrideCursor();
}

/**
	Bars
*/
ParallelCoorBarItem::ParallelCoorBarItem(const QString& field, QGraphicsView * pcv, QGraphicsItem * parent)
	:QGraphicsLineItem(parent), pview(pcv), attachedField(field)
{
	setEnabled(true);
	setAcceptHoverEvents ( true );
}

void ParallelCoorBarItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
	qApp->setOverrideCursor(Qt::PointingHandCursor);
	QPen p = pen();
	p.setColor(Qt::red);
	setPen(p);
}

void ParallelCoorBarItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
	QPen p = pen();
	p.setColor(Qt::gray);
	setPen(p);
	qApp->restoreOverrideCursor();
}

void ParallelCoorBarItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	event->accept();
}

void ParallelCoorBarItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if(QString(pview->metaObject()->className()) == QString("ParallelCoorView") )
	{
		ParallelCoorView *pcv = reinterpret_cast<ParallelCoorView*>(pview);
		pcv->selectField(attachedField);
	}
	qApp->restoreOverrideCursor();
}







