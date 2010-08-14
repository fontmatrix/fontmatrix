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

#include "activationwidgetitem.h"
#include "ui_activationwidgetitem.h"

#include "fmfontdb.h"
#include "fontitem.h"
#include "fmactivate.h"
#include "fmactivationreport.h"

ActivationWidgetItem::ActivationWidgetItem(const QString& fontID, QWidget *parent) :
		QWidget(parent),
		fileName(fontID),
		ui(new Ui::ActivationWidgetItem)
{
	ui->setupUi(this);
	FontItem * f(FMFontDb::DB()->Font(fileName));
	ui->styleName->setText(f->variant());
	ui->activatedStatus->setText(fileName);
	ui->activatedStatus->setChecked(f->isActivated());

	connect(ui->activatedStatus, SIGNAL(toggled(bool)), this, SLOT(activate(bool)));
}

ActivationWidgetItem::~ActivationWidgetItem()
{
	delete ui;
}

void ActivationWidgetItem::changeEvent(QEvent *e)
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


void ActivationWidgetItem::activate(bool a)
{
	FontItem * f(FMFontDb::DB()->Font(fileName));
	if(f == 0)
		return;
	if(a != f->isActivated())
	{
		QList<FontItem*> fl;
		fl.clear();
		fl.append(f);
		FMActivate::getInstance()->errors();
		FMActivate::getInstance()->activate(fl, a);
		QMap<QString,QString> actErr(FMActivate::getInstance()->errors());
		if(actErr.count() > 0)
		{
			FMActivationReport ar(this, actErr);
			ar.exec();
		}
		emit fontStateChanged();
	}
}

void ActivationWidgetItem::changeState(bool s)
{
	ui->activatedStatus->setChecked(s);
}

