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

#include <QInputDialog>

FontCompareWidget::FontCompareWidget(QWidget * parent)
	:QWidget(parent)
{
	setupUi(this);
	
	connect( compareAdd,SIGNAL(clicked()), this, SLOT(addFont()));
	connect( compareRemove,SIGNAL(clicked()), this, SLOT(removeFont()));
	connect( compareFill,SIGNAL(clicked()), this, SLOT(fillChange()));
	connect( compareControls,SIGNAL(clicked()), this, SLOT(controlsChange()));
	connect( compareMetrics,SIGNAL(clicked()), this, SLOT(metricsChange()));
	connect( compareCharSelect,SIGNAL(valueChanged(int)), this, SLOT(characterChange(int)));
}

FontCompareWidget::~ FontCompareWidget()
{
}

void FontCompareWidget::addFont()
{
	QList<FontItem*> fl(typotek::getInstance()->getCurrentFonts());
	QMap<QString, FontItem*> fm;
	foreach(FontItem* f, fl)
	{
		fm[f->path()] = f;
	}
	bool ok;
	QString item = QInputDialog::getItem(this, "Fontmatrix", "Please select a font file", fm.keys() , 0, false, &ok);
	if (ok && !item.isEmpty())
	{
		curFont = fm[item]->path();
		QListWidgetItem* witem = new QListWidgetItem(fm[item]->fancyName());
		witem->setData(Qt::UserRole, fm[item]->path());
		witem->setToolTip(fm[item]->path()); // Here we say: « Deux fois valent mieux qu’une !»
		compareList->addItem(witem);
		
		compareView->changeFont(compareList->row(witem), fm[item]);
		QPixmap px(32,32);
		px.fill(compareView->getColor(compareList->row(witem)));
		witem->setIcon( QIcon(px) );
		compareCharSelect->setRange( fm[item]->firstChar(),  fm[item]->firstChar() +  fm[item]->countChars() );
		compareCharSelect->setValue(fm[item]->firstChar());
	}
}

void FontCompareWidget::removeFont()
{
}

void FontCompareWidget::fillChange()
{
}

void FontCompareWidget::controlsChange()
{
}

void FontCompareWidget::metricsChange()
{
}

void FontCompareWidget::characterChange(int v)
{
	FontItem *f(FMFontDb::DB()->Font(curFont));
	int nc(f->nextChar( f->firstChar() , v) );
	compareView->changeChar(nc);
	compareCharName->setText(QChar(nc));
	
}

void FontCompareWidget::fontChange(QListWidgetItem * witem)
{
	curFont = witem->data(Qt::UserRole).toString();
	FontItem *f(FMFontDb::DB()->Font(curFont));
	compareCharSelect->setRange( f->firstChar(),  f->firstChar() +  f->countChars() );
}
