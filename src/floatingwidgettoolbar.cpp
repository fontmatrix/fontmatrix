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

#include "floatingwidgettoolbar.h"
#include "ui_floatingwidgettoolbar.h"

#include "floatingwidget.h"

FloatingWidgetToolBar::FloatingWidgetToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FloatingWidgetToolBar),
    noClose(false)
{
    ui->setupUi(this);
    ui->closeButton->hide();
    ui->hideButton->hide();
    connect(ui->closeButton, SIGNAL(clicked()), this, SIGNAL(Close()));
    connect(ui->hideButton, SIGNAL(clicked()), this, SIGNAL(Hide()));
    connect(ui->printButton, SIGNAL(clicked()), this, SIGNAL(Print()));
    connect(ui->detachButton, SIGNAL(clicked()), this, SLOT(setDetached()));
}

FloatingWidgetToolBar::~FloatingWidgetToolBar()
{
    delete ui;
}

void FloatingWidgetToolBar::changeEvent(QEvent *e)
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

void FloatingWidgetToolBar::setDetached()
{
	ui->closeButton->show();
	ui->hideButton->show();
	ui->detachButton->hide();

//	QWidget *p(parent());
//	while(0 != p)
//	{
//		if(QString(p->metaObject()->className()) == QString("FloatingWidget"))
//		{
//			FloatingWidget * fw(reinterpret_cast<FloatingWidget*>(p));
//			fw->detach();
//			break;
//		}
//		p = parent();
//	}

	emit Detach();
}

void FloatingWidgetToolBar::setNoClose(bool c)
{
	noClose = c;
	if(noClose)
		ui->closeButton->hide();
	else
		ui->closeButton->show();
}
