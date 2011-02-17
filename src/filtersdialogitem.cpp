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

#include "filtersdialogitem.h"
#include "ui_filtersdialogitem.h"


#include <QMessageBox>

FiltersDialogItem::FiltersDialogItem(const QString& name, const QString& f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FiltersDialogItem),
    filterName(name)
{
    ui->setupUi(this);
    QString ssheet;
    ssheet += QString("QToolButton{border:none;}");
    ssheet += QString("QToolButton:checked{border-bottom:2px solid black;}");
    ssheet += QString("QToolButton:hover{background:white;}");
//	ssheet += QString();
//	ssheet += QString();
//	ssheet += QString();
//	ssheet += QString();
    this->setStyleSheet(ssheet);
//    setButtonsVisible(false);
    ui->filterName->setText(filterName);
    ui->filterName->setToolTip(f);
//    ui->filters->setText(f);

    connect(ui->filterButton, SIGNAL(clicked()), this, SLOT(slotFilter()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(slotRemove()));
}

FiltersDialogItem::~FiltersDialogItem()
{
    delete ui;
}


void FiltersDialogItem::slotFilter()
{
	emit Filter(filterName);
}


void FiltersDialogItem::slotRemove()
{
	if(QMessageBox::question(0, tr("Remove Filter"), tr("Confirm deletion of filter:") + filterName, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Ok)
		emit Remove(filterName);
}

void FiltersDialogItem::setButtonsVisible(bool v)
{
	ui->filterButton->setVisible(v);
	ui->removeButton->setVisible(v);

}

void FiltersDialogItem::enterEvent(QEvent *)
{
//	setButtonsVisible(true);
}

void FiltersDialogItem::leaveEvent(QEvent *)
{
//	setButtonsVisible(false);
}
