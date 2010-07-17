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

#include "filteritem.h"
#include "ui_filteritem.h"
#include "filterdata.h"

FilterItem::FilterItem(FilterData *filter, QWidget *parent) :
		QWidget(parent),
		d(filter),
		ui(new Ui::FilterItem)
{
	ui->setupUi(this);
	ui->filterLabel->setText(d->getText());
	ui->andButton->setChecked(d->data(FilterData::And).toBool());

	connect(ui->andButton, SIGNAL(clicked(bool)), this, SLOT(setAndMode(bool)));
	connect(ui->removeButton, SIGNAL(clicked()), this, SIGNAL(remove()));
}

FilterItem::~FilterItem()
{
	delete ui;
}


void FilterItem::changeEvent(QEvent *e)
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


void FilterItem::setAndMode(bool c)
{
	d->setData(FilterData::And, c, true);
}
