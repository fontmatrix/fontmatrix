//
// C++ Implementation: fontcomparewidget
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fontcomparewidget.h"
#include "fontitem.h"
#include "typotek.h"
#include "fmfontdb.h"

// #define RECORD_MY_REMIX
#ifdef RECORD_MY_REMIX
#include <QImage>
#include <QPainter>
#endif

#include <QSettings>
#include <QDebug>

FontCompareWidget* FontCompareWidget::instance = 0;

FontCompareWidget::FontCompareWidget(QWidget * parent)
	:QWidget(parent),neverUsed(true)
{
	setupUi(this);
	QSettings settings;
	int maxOffset(settings.value("Compare/MaxOffset", 2000).toInt());
	settings.setValue("Compare/MaxOffset",maxOffset);
	compareOffset->setRange(0, maxOffset);
	initColors();
	doconnect();
}

FontCompareWidget::~ FontCompareWidget()
{

}

FontCompareWidget* FontCompareWidget::getInstance()
{
	if(instance == 0)
	{
		instance = new FontCompareWidget(0);
		Q_ASSERT(instance);
	}
	return instance;
}

void FontCompareWidget::initColors()
{
	QSettings settings;
	QStringList defaultColors;
	defaultColors << "aqua" 
			<< "brown" 
			<< "chartreuse" 
			<< "cornflowerblue" 
			<< "darkblue" 
			<< "darkmagenta" 
			<< "olive" 
			<< "darkgrey" 
			<< "mediumvioletred" 
			<< "palevioletred" 
			<< "midnightblue" 
			<< "red" ;
	QPixmap px(32,32);
	compareFillColor->addItem(tr("None", "No fill color in comprae glyph"), "transparent");
	QString colorN("Compare/color%1");
	for(int i(0); i < 12; ++i)
	{
		QString colStr(settings.value(colorN.arg(i), defaultColors[i]).toString());
		QColor col(colStr);
		px.fill(col);
		compareFillColor->addItem(QIcon(px), colStr, col.name());
	}
	for(int i(0); i < 12; ++i)
	{
		QString colStr(settings.value(colorN.arg(i), defaultColors[i]).toString());
		settings.setValue(colorN.arg(i), colStr);// as usual, we write it back to settings so user (me as well ;)) can see it if he opens the config file
	}
}

void FontCompareWidget::doconnect()
{
	connect( compareAdd,SIGNAL(clicked()), this, SLOT(addFont()));
	connect( compareRemove,SIGNAL(clicked()), this, SLOT(removeFont()));
	connect( compareShow,SIGNAL(clicked()), this, SLOT(showChange()));
	connect( compareFillColor,SIGNAL(currentIndexChanged(int)), this, SLOT(fillChange(int)));
	connect( comparePoints,SIGNAL(clicked()), this, SLOT(pointsChange()));
	connect( compareControls,SIGNAL(clicked()), this, SLOT(controlsChange()));
	connect( compareMetrics,SIGNAL(clicked()), this, SLOT(metricsChange()));
	connect( compareOffset, SIGNAL(valueChanged(int)), this, SLOT(offsetChange(int)));
	connect( compareCharSelect,SIGNAL(valueChanged(int)), this, SLOT(characterChange(int)));
	connect( compareCharBox,SIGNAL(currentIndexChanged(int)), this, SLOT(characterBoxChange(int)));
	connect( compareList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fontChange(QListWidgetItem*,QListWidgetItem*)));
	connect( compareSyncChars, SIGNAL(stateChanged( int )), this, SLOT(syncChange(int)));
}

void FontCompareWidget::dodisconnect()
{
	disconnect( compareAdd,SIGNAL(clicked()), this, SLOT(addFont()));
	disconnect( compareRemove,SIGNAL(clicked()), this, SLOT(removeFont()));
	disconnect( compareShow,SIGNAL(clicked()), this, SLOT(showChange()));
	disconnect( compareFillColor,SIGNAL(currentIndexChanged(int)), this, SLOT(fillChange(int)));
	disconnect( comparePoints,SIGNAL(clicked()), this, SLOT(pointsChange()));
	disconnect( compareControls,SIGNAL(clicked()), this, SLOT(controlsChange()));
	disconnect( compareMetrics,SIGNAL(clicked()), this, SLOT(metricsChange()));
	disconnect( compareOffset, SIGNAL(valueChanged(int)), this, SLOT(offsetChange(int)));
	disconnect( compareCharSelect,SIGNAL(valueChanged(int)), this, SLOT(characterChange(int)));
	disconnect( compareCharBox,SIGNAL(currentIndexChanged(int)), this, SLOT(characterBoxChange(int)));
	disconnect( compareList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fontChange(QListWidgetItem*,QListWidgetItem*)));
	disconnect( compareSyncChars, SIGNAL(stateChanged( int )), this, SLOT(syncChange(int)));
}

void FontCompareWidget::resetElements()
{
	compareShow->setChecked(true);
	compareCharBox->adjustSize();
	compareFillColor->setCurrentIndex(0);
	comparePoints->setChecked(false);
	compareControls->setCheckState(Qt::Unchecked);
	compareMetrics->setCheckState(Qt::Unchecked);
	
	compareOffset->setValue(0);
	compareOffsetValue->setText(QString::number(0));
}

void FontCompareWidget::addFont()
{
	FontItem *f(typotek::getInstance()->getSelectedFont());
	if(!f)
		return;
	dodisconnect();
	curFont = f->path();
	QListWidgetItem* witem = new QListWidgetItem(f->fancyName());
	witem->setData(Qt::UserRole, f->path());
	witem->setToolTip(f->path()); // Here we say: « Deux fois valent mieux qu’une !»
	compareList->addItem(witem);
	compareList->setItemSelected(witem, true);
	
	int r(compareList->row(witem));
	compareView->changeFont(compareList->row(witem), f);
	QColor c(compareFillColor->itemData(0).toString());
	compareView->setColor(r, c);
	QPixmap px(32,32);
	px.fill(compareView->getColor(r));
	witem->setIcon( QIcon(px) );
	
	int cn(f->countChars());
	compareCharSelect->setRange( 0 ,  cn );	
	
	compareCharBox->clear();
	int cc(f->firstChar());
	int curCIdx(0);
	for(int co(1); co <= cn; co++)
	{
		if(cc <= curcode)
			curCIdx = co; 
		compareCharBox->addItem(QString("%1  (U+%2)").arg(QChar(cc)).arg(cc,4,16,QChar('0')),cc);
		cc = f->nextChar(cc,1);
	}
	
	if(!neverUsed)
	{
		characterChange(curCIdx - 1);
	}
	else
	{
		compareView->fitGlyphsView();
		characterChange(0);
		neverUsed = false;
	}
	
	resetElements();
	doconnect();
}

void FontCompareWidget::removeFont()
{
	if(compareList->selectedItems().isEmpty())
		return;
	QListWidgetItem *witem(compareList->selectedItems().first());
	int idx(compareList->row(witem));
	
	compareView->removeFont(idx);
	
	resetElements();
	
	delete witem;
	
}

void FontCompareWidget::showChange()
{
	if(compareList->selectedItems().isEmpty())
		return;
	int r(compareList->row(compareList->selectedItems().first()));
	if(compareShow->isChecked())
		compareView->setElements(r, compareView->getElements(r) | FMFontCompareItem::Contour);
	else
		compareView->setElements(r, compareView->getElements(r) ^ FMFontCompareItem::Contour);
}

void FontCompareWidget::fillChange(int newIdx)
{
	if(compareList->selectedItems().isEmpty())
		return;
	QListWidgetItem * witem(compareList->selectedItems().first());
	int r(compareList->row(witem));
	QString colorString(compareFillColor->itemData( newIdx ).toString());
	QColor color(colorString);
	compareView->setColor(r,color);
	
	QPixmap px(32,32);
	px.fill(color);
	witem->setIcon( QIcon(px) );
	
}

void FontCompareWidget::pointsChange()
{
	if(compareList->selectedItems().isEmpty())
		return;
	int r(compareList->row(compareList->selectedItems().first()));
	if(comparePoints->isChecked())
		compareView->setElements(r, compareView->getElements(r) | FMFontCompareItem::Points);
	else
		compareView->setElements(r, compareView->getElements(r) ^ FMFontCompareItem::Points);
}

void FontCompareWidget::controlsChange()
{
	if(compareList->selectedItems().isEmpty())
		return;
	int r(compareList->row(compareList->selectedItems().first()));
	if(compareControls->isChecked())
		compareView->setElements(r, compareView->getElements(r) | FMFontCompareItem::Controls);
	else
		compareView->setElements(r, compareView->getElements(r) ^ FMFontCompareItem::Controls);
}

void FontCompareWidget::metricsChange()
{
	if(compareList->selectedItems().isEmpty())
		return;
	int r(compareList->row(compareList->selectedItems().first()));
	if(compareMetrics->isChecked())
		compareView->setElements(r, compareView->getElements(r) | FMFontCompareItem::Metrics);
	else
		compareView->setElements(r, compareView->getElements(r) ^ FMFontCompareItem::Metrics);
}

void FontCompareWidget::offsetChange(int o)
{
	if(compareList->selectedItems().isEmpty())
		return;
	int r(compareList->row(compareList->selectedItems().first()));
	compareView->setOffset(r, o);	
	compareOffsetValue->setText(QString::number(o));
}

void FontCompareWidget::characterChange(int v)
{
// 	qDebug()<<"FontCompareWidget::characterChange"<<v;
	FontItem *f(FMFontDb::DB()->Font(curFont));
	if(!f)
		return;
// 	int nc(0);
	if(v < 0)
	{
// 		nc = curcode;
	}
	else if(v == 0) // someone  asked for first char, just avoid a white space!
	{
		do
		{
			compareCharSelect->setValue(v);
			compareCharBox->setCurrentIndex(v);
			curcode = f->nextChar( f->firstChar() , v);
			++v;
			
		}while(f->legitimateNonPathChars.contains(curcode));
		--v;
	}
	else
	{
		curcode = f->nextChar( f->firstChar() , v);
	}
// 	qDebug()<<"V"<<v;
	compareCharBox->setCurrentIndex(v);
	
	if(compareSyncChars->isChecked())
		compareView->changeChar(curcode);
	else
	{
		if(!compareList->selectedItems().isEmpty())
		{
			int r(compareList->row(compareList->selectedItems().first()));
			compareView->changeChar(r,curcode);
		}
	}
	
// 	compareCharName->setText(QChar(nc));
#ifdef RECORD_MY_REMIX
	// a bit of fun!
	QImage img(compareView->size(), QImage::Format_ARGB32);
	QPainter p(&img);
	p.setRenderHint(QPainter::Antialiasing,true);
	compareView->render(&p);
	
	QString ip(QString("cmp-%1.png").arg(curcode,8, 10,  QChar('0') ));
	img.save(ip.toLocal8Bit());
	
#endif
	
}

void FontCompareWidget::characterBoxChange(int i)
{
// 	qDebug()<<"FontCompareWidget::characterBoxChange"<<i;
	FontItem *f(FMFontDb::DB()->Font(curFont));
	if(!f)
		return;
	
	compareCharSelect->setValue(i);
	
	int nc(compareCharBox->itemData(i).toInt());
	curcode = nc;
	if(compareSyncChars->isChecked())
		compareView->changeChar(curcode);
	else
	{
		if(!compareList->selectedItems().isEmpty())
		{
			int r(compareList->row(compareList->selectedItems().first()));
			compareView->changeChar(r,curcode);
		}
	}
}

void FontCompareWidget::fontChange(QListWidgetItem * witem, QListWidgetItem * olditem)
{
	if(!witem)
	{
		resetElements();
		return;
	}
	dodisconnect();
	resetElements();
	
	curFont = witem->data(Qt::UserRole).toString();
	FontItem *f(FMFontDb::DB()->Font(curFont));
	int cn(f->countChars());
	compareCharBox->clear();
	int curCIdx(0);
	int cc(f->firstChar());
	for(int co(1); co <= cn; co++)
	{
		compareCharBox->addItem( QString("%1  (U+%2)").arg(QChar(cc)).arg(cc,4,16,QChar('0')),cc);
		if(cc < curcode)
		{
			curCIdx = co;
		}
		cc = f->nextChar(cc,1);
	}
	compareCharBox->setCurrentIndex(curCIdx);
	compareCharSelect->setRange( 0,  cn );
	compareCharSelect->setValue(curCIdx);

	
	
	int r(compareList->row(witem));
	FMFontCompareItem::GElements e(compareView->getElements(r));
	
	compareOffset->setValue(qRound(compareView->getOffset(r)));
	compareOffsetValue->setText(QString::number(compareView->getOffset(r)));
	
	QString colStr(compareView->getColor(r).name());
	int colIdx(compareFillColor->findData(colStr));
	compareFillColor->setCurrentIndex(colIdx);
	
	if(e.testFlag(FMFontCompareItem::Contour))
	{
		compareShow->setChecked(true);
	}
	else
	{
		compareShow->setChecked(false);
	}
	if(e.testFlag(FMFontCompareItem::Points))
	{
		comparePoints->setChecked(true);
	}
	else
	{
		comparePoints->setChecked(false);
	}
	
	if(e.testFlag(FMFontCompareItem::Controls))
	{
		compareControls->setCheckState(Qt::Checked);
	}
	else
	{
		compareControls->setCheckState(Qt::Unchecked);
	}
	
	if(e.testFlag(FMFontCompareItem::Metrics))
	{
		compareMetrics->setCheckState(Qt::Checked);
	}
	else
	{
		compareMetrics->setCheckState(Qt::Unchecked);
	}
	
	doconnect();
}

void FontCompareWidget::syncChange(int state)
{
	if((state == Qt::Checked) && (!neverUsed))
	{
		characterChange(-1);
	}
}


