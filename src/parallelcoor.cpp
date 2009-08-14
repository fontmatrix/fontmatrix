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
#include <QFontMetricsF>
#include <QSettings>
#include <QVariant>
#include <QColor>

/**
	DataSet
*/
const QString ParallelCoorDataSet::FieldSep = ":";

ParallelCoorDataSet::ParallelCoorDataSet()
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
QMap<QString, QPen> ParallelCoorView::pens;
QMap<QString, QBrush> ParallelCoorView::brushes;
QPainterPath ParallelCoorView::markPath;

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
	setBackgroundBrush(Qt::white);
	initPensAndBrushes();
	doConnect();
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
	setBackgroundBrush(Qt::white);
	initPensAndBrushes();
	doConnect();
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
	if(theValue!=m_currentField)
	{
		m_currentField = theValue;
		cleanLists(ValueList);
		drawValues();
	}
}

ParallelCoorDataSet* ParallelCoorView::getDataSet() const
{
	return m_dataSet;
}


void ParallelCoorView::setDataSet ( ParallelCoorDataSet* theValue )
{
	if(m_dataSet && (m_dataSet != theValue))
		delete m_dataSet;
	m_dataSet = theValue;
}

ParallelCoorView::Units::Units(int width, int height, int count)
{			
	hunit = static_cast<double> ( height ) /1000.0 ;
	wunit = static_cast<double> ( width ) /1000.0  ;
	XOffset = wunit * 200.0;
	YOffset = hunit * 100.0 ;
	H = hunit * 800.0 ;
	W = wunit * 700.0 ;
	C = count ;
	step = W / static_cast<double> ( C-1 ) ;
}

void ParallelCoorView::initPensAndBrushes()
{
	// bars
	pens["bar"] = QPen(QColor(200,200,200), 6.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	pens["bar-hover"] = QPen(QColor(160,160,160), 6.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	
	// vertices
	pens["vertice-filter"] = QPen(Qt::black, 1.0);
	pens["vertice-unfilter"] = QPen(QColor(200,200,200), 1.0);
	
	// marks
	double size(0.5);
	markPath.addRect(-10.0*size,-10.0*size,20.0*size,20.0*size);
	brushes["mark"] = QBrush(Qt::black);
	brushes["mark-active"] = QBrush(Qt::red);
	
	// debug
	pens["debug-1"] = QPen(Qt::blue, 5.0);
	
	QString cat("Panose/color-%1");
	QSettings settings;
	foreach(QString attr, pens.keys())
	{
		pens[attr].setColor( QColor(settings.value(cat.arg(attr),pens[attr].color().name()).toString()) );
		settings.setValue(cat.arg(attr),pens[attr].color().name());
	}
	foreach(QString attr, brushes.keys())
	{
		brushes[attr].setColor( QColor(settings.value(cat.arg(attr),brushes[attr].color().name()).toString()) );
		settings.setValue(cat.arg(attr),pens[attr].color().name());
	}
}

void ParallelCoorView::cleanLists(ItemList il)
{
	if((il == AllList) || (il == ValueList))
	{
		foreach(ParallelCoorValueItem* ti, valueLabels)
		{
			delete ti;
		}
		valueLabels.clear();
		
		foreach(ParallelCoorMarkItem *mi, marks)
		{
			delete mi;
		}
		marks.clear();
	}
	if((il == AllList) || (il == FieldList))
	{
		foreach(ParallelCoorFieldItem* ti, fieldLabels)
		{
			delete ti;
		}
		fieldLabels.clear();
	}
	if((il == AllList) || (il == VerticeList))
	{
		foreach(QGraphicsLineItem* pi, vertices)
		{
			delete pi;
		}
		vertices.clear();
	}
	if((il == AllList) || (il == BarList))
	{
		foreach(ParallelCoorBarItem* li, bars)
		{
			delete li;
		}
		bars.clear();
	}
	
}

void ParallelCoorView::redraw()
{
// 	qDebug()<<"ParallelCoorView::redraw";
	if ( !m_dataSet )
	{
		qWarning()<<"No Dataset";
		return;
	}
	if ( m_dataSet->isEmpty() )
	{
		qWarning()<<"Empty Dataset";
		return;
	}
	if(controlSize != size())
	{
		// there will be another resize event soon, no need to draw now
		return;
	}
	
	units = Units(width(), height(), m_dataSet->count());
// 	QTime t;
// 	int tclean, tbar, tvert, tfield, tval;
// 	t.start();
	cleanLists(AllList);
// // 	tclean = t.elapsed();
// 	t.start();
	drawBars();
// 	tbar = t.elapsed();
// 	t.start();
	drawVertices();
// // 	tvert = t.elapsed();
// // 	t.start();
	drawFields();
// 	tfield = t.elapsed();
// 	t.start();
	drawValues();
// 	tval = t.elapsed();

// 	qDebug()<<"C"<<tclean<<"B"<<tbar<<"Ve"<<tvert<<"F"<<tfield<<"Va"<<tval;
}

void ParallelCoorView::drawBars()
{
	for ( int i ( 0 ); i < units.C ; ++i )
	{
		double di ( static_cast<double> ( i ) );
		QLineF bl(units.XOffset + ( di * units.step ),  units.YOffset,
			  units.XOffset + ( di * units.step ),  units.YOffset + units.H);
		ParallelCoorBarItem * bi ( new ParallelCoorBarItem(m_dataSet->at(i).first, this) );
		bars << bi;
		bi->setPen(pens["bar"]);
		bi->setLine(bl);
		scene()->addItem(bi);
	}

}

void ParallelCoorView::drawVertices()
{
// 	qDebug()<<this<<"::drawVertices"<<m_dataSet->getData().count();
	int tc, td, ta, to;
	tc = td = ta = to = 0;
	QTime t;
	t.start();
	const int N ( m_dataSet->getData().count() );
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
// 	QList<QLineF> cflines;
// 	QList<QLineF> culines;
	QMap<double, QMap<double ,QList<QPointF> > > cflines;
	QMap<double, QMap<double ,QList<QPointF> > > culines;
	to = t.elapsed();		
	QGraphicsLineItem *li;
	for ( int a ( 0 );a<N;++a )
	{
// 		if( m_dataSet->getData().at(a).count() == m_dataSet->count() )
		{
			QList<QPointF> pol;
			t.start();
			for ( int b ( 0 ); b < m_dataSet->getData().at ( a ).count() ; ++b )
			{
				if(placeCoords[b].contains( m_dataSet->getData().at ( a ).at ( b ) ))
					pol << QPointF( placeCoords[b][m_dataSet->getData().at ( a ).at ( b ) ] );
				else
					pol << QPointF( placeCoords[b][0] );

			}
			tc += t.elapsed();
			t.start();
			if ( pol.count() == m_dataSet->count() )
			{
				for(int vi(1);vi<pol.count();++vi)
				{
					bool f ( matchFilter ( m_dataSet->getData().at ( a ) ) );
					if(f)
					{
						if(cflines.contains(pol[vi-1].x()))
						{
							if(cflines[pol[vi-1].x()].contains(pol[vi-1].y()))
							{
								if(cflines[pol[vi-1].x()][pol[vi-1].y()].contains(pol[vi]))
								{
									continue;
								}
							}
						}
						QLineF lf(pol[vi-1],pol[vi]);
						li = new QGraphicsLineItem( lf );
						li->setPen(  pens["vertice-filter"] );
						li->setZValue(100.0);
						vertices << li;
						cflines[pol[vi-1].x()][pol[vi-1].y()] << pol[vi];
					}
					else
					{
						if(culines.contains(pol[vi-1].x()))
						{
							if(culines[pol[vi-1].x()].contains(pol[vi-1].y()))
							{
								if(culines[pol[vi-1].x()][pol[vi-1].y()].contains(pol[vi]))
								{
									continue;
								}
							}
						}
						QLineF lf(pol[vi-1],pol[vi]);
						li = new QGraphicsLineItem( lf );
						li->setPen(  pens["vertice-unfilter"] );
						vertices << li;
						culines[pol[vi-1].x()][pol[vi-1].y()] << pol[vi];
					}
				}
			}
			td += t.elapsed();
			
		}
	}
	t.start();
	int vcount(vertices.count());
	QGraphicsScene * ls(scene());
	for(int i(0); i < vcount; ++i)
	{
		ls->addItem( vertices[i] );
	}
	ta = t.elapsed();
	qDebug()<<"R"<< to << tc << td << ta;
}

void ParallelCoorView::drawFields()
{
// 	qDebug()<<"ParallelCoorView::drawFields";
	QFont fontF( "Helvetica" , 100.0 , QFont::DemiBold, false );
	double lastW(0.0);
	bool lastPosShifted(false);
	double maxAscent(0.0);
	for(int k(0);k < m_dataSet->count(); ++k)
	{
		QString f(m_dataSet->at(k).first);
		QFontMetricsF metrics(fontF);
		double w(metrics.boundingRect(f).width());
		double fsize( units.step * 100.0 / w );
		double ascent(metrics.ascent() * fsize / 100.0 );
		maxAscent = qMax(maxAscent, ascent);
	}
	for(int k(0);k < m_dataSet->count(); ++k)
	{
		QString f(m_dataSet->at(k).first);
		
		fontF.setPointSizeF(100.0);
		QFontMetricsF metrics(fontF);
		double w(metrics.boundingRect(f).width());
		double fsize( (units.step * 0.9) * 100.0 / w );
		double ascent(metrics.ascent() * fsize / 100.0);
		double sw(w * fsize / 100.0 );
		fontF.setPointSizeF(fsize);
		
		ParallelCoorFieldItem * ti = new ParallelCoorFieldItem(f,this);
		fieldLabels << ti;
		ti->setFont(fontF);
		scene()->addItem(ti);
		ti->setPos(units.XOffset + (k*units.step) - (sw/2.0), units.H + units.YOffset + (maxAscent - ascent));
// 		double h(ti->boundingRect().height());
// 		if(lastW > units.step * 1.1)
// 		{
// 			if(!lastPosShifted)
// 			{
// 				ti->setPos(units.XOffset + (k*units.step) , units.H + units.YOffset + h);
// 				lastPosShifted = true;
// 			}
// 			else
// 			{
// 				ti->setPos(units.XOffset + (k*units.step) , units.H + units.YOffset );
// 				lastPosShifted = false;
// 			}
// 		}
// 		else
// 		{
// 			ti->setPos(units.XOffset + (k*units.step) /*- (w/3.0)*/, units.H+units.YOffset);
// 			lastPosShifted = false;			
// 		}
// 		lastW = w;
	}
	
}

void ParallelCoorView::drawValues()
{
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
	
	QFont fontV( "Helvetica" , 9, QFont::Normal , true );
	QFont fontS( "Helvetica" , 10, QFont::DemiBold , true );
	double din(static_cast<double>(di));
	double dn(static_cast<double>(m_dataSet->at(di).second.count()-1));
	double vsep(units.H / dn);
	QList<QString> list (m_dataSet->at(di).second);
	for(int i(0); i< list.count(); ++i)
	{
		ParallelCoorValueItem *vi = new ParallelCoorValueItem(list[i], this);
		valueLabels << vi;
		ParallelCoorMarkItem *mi = new ParallelCoorMarkItem(vi, this);
		marks << mi;
		if(cfilter.contains(di))
		{
			if(cfilter[di].contains(i))
				vi->setFont(fontS);
			else
				vi->setFont(fontV);
		}
		else
			vi->setFont(fontV);
		
		scene()->addItem(vi);
		scene()->addItem(mi);
		
		double w((units.XOffset * .9) - vi->boundingRect().width());
		double h(vi->boundingRect().height() / 2.0);
// 		qDebug()<<"V"<<w<<vi->boundingRect().width()<<units.XOffset;
		vi->setPos( w , units.YOffset + (static_cast<double>(i) * vsep) - h);
		mi->setPos( units.XOffset + ( static_cast<double>(di) * units.step ),  units.YOffset + (static_cast<double>(i) * vsep));
		vi->setZValue(1000.0);
		mi->setZValue(1000.0);
// 		qDebug()<<"=========================================================";
// 		qDebug()<<vi;
// 		qDebug()<<mi;
// 		qDebug()<<"=========================================================";
	}
}

void ParallelCoorView::updateGraphic()
{
	if(isVisible())
		redraw();
}

void ParallelCoorView::resizeEvent(QResizeEvent * event)
{
	controlSize = size();
	updateGraphic();
	QGraphicsView::resizeEvent(event);
}

void ParallelCoorView::showEvent(QShowEvent * event)
{
// 	updateGraphic();
	QGraphicsView::showEvent(event);
}


QMap< QString, QStringList > ParallelCoorView::getFilter() const
{
	return m_filter;
}

QString ParallelCoorView::filterAsString()
{
	QString ret;
	foreach(QString key, m_filter.keys())
	{
		const QStringList& l = m_filter[key];
		if(!l.isEmpty())
		{
			if(ret.isEmpty())
				ret += key + " {" + l.join(";") + "}";
			else
				ret += "\n" + key + " {" + l.join(";") + "}";
		}
			
	}
	return ret;
	
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
	updateGraphic();
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

void ParallelCoorValueItem::hoverEnter()
{
	qApp->setOverrideCursor(Qt::PointingHandCursor);
	QBrush b = brush();
	b.setColor(Qt::red);
	setBrush(b);
}

void ParallelCoorValueItem::hoverLeave()
{
	QBrush b = brush();
	b.setColor(Qt::black);
	setBrush(b);
	qApp->restoreOverrideCursor();
}

void ParallelCoorValueItem::click(int mod)
{
	if(QString(pview->metaObject()->className()) == QString("ParallelCoorView") )
	{
		ParallelCoorView *pcv = reinterpret_cast<ParallelCoorView*>(pview);
		
		QMap<QString, QStringList> filter;
		if(mod == 0) // bare left click
		{
			filter[pcv->getCurrentField()] << text();
		}
		else if(mod == 1) // with Shift
		{
			filter = pcv->getFilter();
			filter[pcv->getCurrentField()] << text();
		}
		else if(mod == 2) // with Control
		{
			filter = pcv->getFilter();
			filter[pcv->getCurrentField()].removeAll(text());
		}
		
		pcv->setFilter(filter);
	}
	qApp->restoreOverrideCursor();
}

void ParallelCoorValueItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
	hoverEnter();
}

void ParallelCoorValueItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
	hoverLeave();
}

void ParallelCoorValueItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	event->accept();
}

void ParallelCoorValueItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if(event->modifiers() & Qt::ShiftModifier)
		click(1);
	else if(event->modifiers() & Qt::ControlModifier)
		click(2);
	else
		click();
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
// 	qApp->setOverrideCursor(Qt::PointingHandCursor);
	setPen(ParallelCoorView::pens["bar-hover"]);
	if(QString(pview->metaObject()->className()) == QString("ParallelCoorView") )
	{
		ParallelCoorView *pcv = reinterpret_cast<ParallelCoorView*>(pview);
		pcv->selectField(attachedField);
	}
}

void ParallelCoorBarItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
	setPen(ParallelCoorView::pens["bar"]);
// 	qApp->restoreOverrideCursor();
}

void ParallelCoorBarItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
// 	event->accept();
}

void ParallelCoorBarItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
// 	if(QString(pview->metaObject()->className()) == QString("ParallelCoorView") )
// 	{
// 		ParallelCoorView *pcv = reinterpret_cast<ParallelCoorView*>(pview);
// 		pcv->selectField(attachedField);
// 	}
// 	qApp->restoreOverrideCursor();
}

/**
	Marks
*/

ParallelCoorMarkItem::ParallelCoorMarkItem(ParallelCoorValueItem * relative, QGraphicsView * pcv, QGraphicsItem * parent)
	:QGraphicsPathItem(parent), pview(pcv), value(relative)
{
	setEnabled(true);
	setAcceptHoverEvents ( true );
	
	setBrush(ParallelCoorView::brushes["mark"]);
	setPen(QPen(Qt::transparent,0.0));
	setPath(ParallelCoorView::markPath);
	
}

void ParallelCoorMarkItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
	setBrush(ParallelCoorView::brushes["mark-active"]);
	value->hoverEnter();
	QGraphicsPathItem::hoverEnterEvent(event);
}

void ParallelCoorMarkItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
	setBrush(ParallelCoorView::brushes["mark"]);
	value->hoverLeave();
	QGraphicsPathItem::hoverLeaveEvent(event);
}

void ParallelCoorMarkItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	event->accept();
}

void ParallelCoorMarkItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if(event->modifiers() & Qt::ShiftModifier)
		value->click(1);
	else if(event->modifiers() & Qt::ControlModifier)
		value->click(2);
	else
		value->click();
}

void ParallelCoorView::slotSaveColors()
{
// 	qDebug()<<"ParallelCoorView::~ParallelCoorView()";
// 	QString cat("Panose/%1");
// 	QSettings settings;
// 	foreach(QString attr, pens.keys())
// 	{
// 		qDebug()<<cat.arg(attr);
// 		settings.setValue(cat.arg(attr),pens[attr].color() );
// 	}
// 	foreach(QString attr, brushes.keys())
// 	{
// 		qDebug()<<cat.arg(attr);
// 		settings.setValue(cat.arg(attr),brushes[attr].color() );
// 	}
}

void ParallelCoorView::doConnect()
{
// 	connect(this, SIGNAL(destroyed( QObject* )), this, SLOT(slotSaveColors()));
}










