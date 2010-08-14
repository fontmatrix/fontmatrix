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

#include "activationwidget.h"
#include "ui_activationwidget.h"

#include "fmfontdb.h"
#include "fmactivate.h"
#include "fmactivationreport.h"
#include "fontitem.h"
#include "activationwidgetitem.h"
#include "fmvariants.h"


const QString ActivationWidget::Name = QObject::tr("Activation");

ActivationWidget::ActivationWidget(const QString& familyName, QWidget *parent) :
		FloatingWidget(familyName, Name, parent),
		family(familyName),
		ui(new Ui::ActivationWidget)
{
	ui->setupUi(this);
	QList<FontItem*> fl(FMVariants::Order(FMFontDb::DB()->FamilySet(family)));
	foreach(FontItem* f, fl)
	{
		ActivationWidgetItem * i(new ActivationWidgetItem(f->path(), this));
		ui->listLayout->addWidget(i);
		items.append(i);

		connect(i, SIGNAL(fontStateChanged()), this, SIGNAL(familyStateChanged()));
	}

	connect(ui->activateAll, SIGNAL(clicked()), this, SLOT(slotActivate()));
	connect(ui->deactivateAll, SIGNAL(clicked()), this, SLOT(slotDeactivate()));
}

ActivationWidget::~ActivationWidget()
{
	delete ui;
}

void ActivationWidget::changeEvent(QEvent *e)
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

void ActivationWidget::slotActivate()
{
	activateAll(true);
}

void ActivationWidget::slotDeactivate()
{
	activateAll(false);
}

void ActivationWidget::activateAll(bool c)
{
	FMActivate::getInstance()->errors();
	FMActivate::getInstance()->activate(FMFontDb::DB()->FamilySet(family), c);
	QMap<QString,QString> actErr(FMActivate::getInstance()->errors());
	if(actErr.count() > 0)
	{
		FMActivationReport ar(this, actErr);
		ar.exec();
	}
	foreach(ActivationWidgetItem *i, items)
	{
		i->changeState(c);
	}

	emit familyStateChanged();
}
