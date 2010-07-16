/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "filterbar.h"
#include "ui_filterbar.h"
#include "fmfontdb.h"
#include "panosewidget.h"
#include "metawidget.h"
#include "filteritem.h"
#include "filtertag.h"
#include "filterpanose.h"
#include "filtermeta.h"

#include <QDialog>
#include <QGridLayout>
#include <QStringList>

FilterBar::FilterBar(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::FilterBar)
{
	ui->setupUi(this);
	ui->filterListBar->hide();
	loadTags();

	connect(ui->classButton, SIGNAL(clicked()), this, SLOT(panoseDialog()));
	connect(ui->metaButton, SIGNAL(clicked()), this, SLOT(metaDialog()));
	connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(slotClearFilter()));
	connect(PanoseWidget::getInstance(), SIGNAL(filterChanged()), this, SLOT(slotPanoFilter()));
	connect(ui->tagsCombo, SIGNAL(activated(const QString&)), this, SLOT(slotTagSelect(const QString&)));
}

FilterBar::~FilterBar()
{
	delete ui;
}

void FilterBar::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void FilterBar::loadTags()
{
	ui->tagsCombo->clear();
	//	tagsetIcon = QIcon(":/fontmatrix_tagseteditor.png");

	ui->tagsCombo->addItem(tr("Tags"),"NO_KEY");
	ui->tagsCombo->addItem(tr("All activated"),"ALL_ACTIVATED");
	//	ui->tagsCombo->addItem(tr("Similar to current"),"SIMILAR");

	QStringList tl_tmp = FMFontDb::DB()->getTags();
	tl_tmp.sort();
	foreach(QString tag, tl_tmp )
	{
		ui->tagsCombo->addItem(tag, "TAG");
	}

}

void FilterBar::panoseDialog()
{
	PanoseWidget* pw(PanoseWidget::getInstance());
	pw->show();
}

void FilterBar::metaDialog()
{
	MetaWidget * mw(new MetaWidget);
	QDialog *d = new QDialog(this);
	QGridLayout *l = new QGridLayout(d);
	l->addWidget(mw,0,0,0,0);

	connect(mw,SIGNAL(filterAdded()), d, SLOT(close()));

	d->exec();
	if((mw->resultField != -1) && (!mw->resultText.isEmpty()))
	{
//		emit initSearch(mw->resultField, mw->resultText);
		FilterMeta *fm(new FilterMeta);
		fm->setData(FilterData::Text, mw->resultText);
		fm->setData(FilterMeta::Field, mw->resultField);
		fm->setData(FilterMeta::Value, mw->resultText);
		addFilter(fm);
	}
	delete l;
	delete d;

}

void FilterBar::processFilters()
{
	FMFontDb::DB()->clearFilteredFonts();
	foreach(FilterItem* d, filters)
	{
		d->filter()->operate();
	}
	emit filterChanged();
}

void FilterBar::slotRemoveFilter(bool process)
{
	FilterItem * fi(reinterpret_cast<FilterItem*>(sender()));
	if(fi != 0)
	{
		delete fi->filter();
		filters.removeAll(fi);
		if(filters.count() == 0)
		{
			ui->filterListBar->hide();
			FMFontDb::DB()->filterAllFonts();
			emit filterChanged();
		}
		fi->deleteLater();
		if(process && (filters.count() > 0) )
			processFilters();
	}
}

void FilterBar::removeAllFilters()
{
	FMFontDb::DB()->filterAllFonts();
	foreach(FilterItem* d, filters)
	{
		d->deleteLater();
	}
	filters.clear();
	ui->filterListBar->hide();
}

void FilterBar::addFilter(FilterData *f)
{
	if(f != 0)
	{
		FilterItem * it(f->item());
		if(!filters.contains(it))
		{
			if(f->data(FilterData::Replace).toBool())
				removeAllFilters();
			connect(it, SIGNAL(remove()), this, SLOT(slotRemoveFilter()));
			filters.append(it);
			ui->filterListLayout->addWidget(it);
			if(!(ui->filterListBar->isVisible()))
				ui->filterListBar->show();

			processFilters();
		}
	}
}

void FilterBar::slotTagSelect(const QString& t)
{
	QString key(ui->tagsCombo->itemData(ui->tagsCombo->currentIndex()).toString());
	ui->tagsCombo->setCurrentIndex(0);
	FilterTag * ft(new FilterTag);
	ft->setData(FilterData::Text, t);
	ft->setData(FilterTag::Key, key);
	ft->setData(FilterTag::Tag, t);
	addFilter(ft);
}

void FilterBar::slotPanoFilter()
{
	QMap<int,QList<int> > pv(PanoseWidget::getInstance()->getFilter());
	const QMap< FontStrings::PanoseKey, QMap<int, QString> >& ps(FontStrings::Panose());
	foreach(int k, pv.keys())
	{
		foreach(int v, pv[k])
		{
			FontStrings::PanoseKey pk (static_cast<FontStrings::PanoseKey>(k));
			QString text(FontStrings::PanoseKeyName(pk) + QString(" : ") + ps.value(pk).value(v));
			FilterPanose *fp(new FilterPanose);
			fp->setData(FilterData::Text, text);
			fp->setData(FilterPanose::Param, k);
			fp->setData(FilterPanose::Value, v);
			addFilter(fp);
		}
	}

}

void FilterBar::slotClearFilter()
{
	removeAllFilters();
	emit filterChanged();
}
