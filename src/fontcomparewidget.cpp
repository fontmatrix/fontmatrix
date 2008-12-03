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

FontCompareWidget::FontCompareWidget(QWidget * parent)
	:QWidget(parent),neverUsed(true)
{
	setupUi(this);
	QSettings settings;
	int maxOffset(settings.value("Compare/MaxOffset", 2000).toInt());
	settings.setValue("Compare/MaxOffset",maxOffset);
	compareOffset->setRange(0, maxOffset);
	doconnect();
}

FontCompareWidget::~ FontCompareWidget()
{
	
}
void FontCompareWidget::doconnect()
{
	connect( compareAdd,SIGNAL(clicked()), this, SLOT(addFont()));
	connect( compareRemove,SIGNAL(clicked()), this, SLOT(removeFont()));
	connect( compareShow,SIGNAL(clicked()), this, SLOT(showChange()));
	connect( compareFill,SIGNAL(clicked()), this, SLOT(fillChange()));
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
	disconnect( compareFill,SIGNAL(clicked()), this, SLOT(fillChange()));
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
	compareFill->setCheckState(Qt::Unchecked);
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
	
	compareView->changeFont(compareList->row(witem), f);
	QPixmap px(32,32);
	px.fill(compareView->getColor(compareList->row(witem)));
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

void FontCompareWidget::fillChange()
{
	if(compareList->selectedItems().isEmpty())
		return;
	int r(compareList->row(compareList->selectedItems().first()));
	if(compareFill->isChecked())
		compareView->setElements(r, compareView->getElements(r) | FMFontCompareItem::Fill);
	else
		compareView->setElements(r, compareView->getElements(r) ^ FMFontCompareItem::Fill);
	
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
	
	if(e.testFlag(FMFontCompareItem::Contour))
	{
		compareShow->setChecked(true);
	}
	else
	{
		compareShow->setChecked(false);
	}
	if(e.testFlag(FMFontCompareItem::Fill))
	{
		compareFill->setCheckState(Qt::Checked);
	}
	else
	{
		compareFill->setCheckState(Qt::Unchecked);
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


