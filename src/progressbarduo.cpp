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

#include "progressbarduo.h"
#include "ui_progressbarduo.h"

ProgressBarDuo::ProgressBarDuo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressBarDuo)
{
    ui->setupUi(this);
    connect(ui->cancelButton, SIGNAL(Canceled()), this, SIGNAL(Canceled()));
}

ProgressBarDuo::~ProgressBarDuo()
{
    delete ui;
}

void ProgressBarDuo::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void ProgressBarDuo::setLabel(const QString &s, int n)
{
	QLabel *l = 0;

	switch(n)
	{
	case 0: l = ui->Label0;
	break;
	case 1: l = ui->Label1;
	break;
	default: break;
	}
	if(l)
		l->setText(s);
}


void ProgressBarDuo::setValue(int value, int n)
{
	QProgressBar *p = 0;
	switch(n)
	{
	case 0: p = ui->Bar0;
	break;
	case 1: p = ui->Bar1;
	break;
	default: break;
	}
	if(p)
		p->setValue(value);
}

void ProgressBarDuo::setMax(int max, int n)
{
	QProgressBar *p = 0;
	switch(n)
	{
	case 0: p = ui->Bar0;
	break;
	case 1: p = ui->Bar1;
	break;
	default: break;
	}
	if(p)
		p->setMaximum(max);
}
