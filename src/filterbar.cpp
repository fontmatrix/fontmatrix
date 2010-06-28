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

#include <QDialog>
#include <QGridLayout>
#include <QStringList>

FilterBar::FilterBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterBar)
{
    ui->setupUi(this);
    loadTags();

    connect(ui->classButton, SIGNAL(clicked()), this, SLOT(panoseDialog()));
    connect(ui->metaButton, SIGNAL(clicked()), this, SLOT(metaDialog()));
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


QComboBox* FilterBar::tagsCombo()
{
	return ui->tagsCombo;
}

QPushButton* FilterBar::clearButton()
{
	return ui->clearButton;
}

void FilterBar::panoseDialog()
{
	PanoseWidget* pw(PanoseWidget::getInstance());
	QDialog *d = new QDialog(this);
	QGridLayout *l = new QGridLayout(d);
	l->addWidget(pw,0,0,0,0);
	d->exec();
	pw->setParent(0);
	delete l;
	delete d;

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
		emit initSearch(mw->resultField, mw->resultText);
	delete l;
	delete d;

}
