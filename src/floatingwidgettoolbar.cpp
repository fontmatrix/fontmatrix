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

FloatingWidgetToolBar::FloatingWidgetToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FloatingWidgetToolBar),
    noClose(false)
{
    ui->setupUi(this);
    connect(ui->closeButton, SIGNAL(clicked()), this, SIGNAL(Close()));
    connect(ui->hideButton, SIGNAL(clicked()), this, SIGNAL(Hide()));
    connect(ui->printButton, SIGNAL(clicked()), this, SIGNAL(Print()));
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


void FloatingWidgetToolBar::setNoClose(bool c)
{
	noClose = c;
	if(noClose)
		ui->closeButton->hide();
	else
		ui->closeButton->show();
}
