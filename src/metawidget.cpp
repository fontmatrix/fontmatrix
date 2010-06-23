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

#include "metawidget.h"
#include "ui_metawidget.h"


#include <QStringListModel>
#include <QCompleter>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

QStringListModel * MetaWidget::mModel = 0;

MetaWidget::MetaWidget(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::MetaWidget)
{
	ui->setupUi(this);
	if(mModel = 0)
	{
		mModel = new QStringListModel;
	}

	QGridLayout * grid(new QGridLayout(this));
	QCompleter * completer(new QCompleter(mModel));
//	ui->setLayout(grid);

	for(int gIdx(0); gIdx < FontStrings::Names().keys().count() ; ++gIdx)
	{
		FMFontDb::InfoItem k(FontStrings::Names().keys()[gIdx]);
		if((k !=  FMFontDb::AllInfo))
		{
			QLabel *label(new QLabel(FontStrings::Names().value(k),this));
			QLineEdit *line(new QLineEdit(this));
			line->setCompleter(completer);
			QPushButton * button(new QPushButton(tr("Add"), this));
			formFieldButton[button] = k;
			formFieldLine[k] = line;
			label->setBuddy(line);
			grid->addWidget(label,gIdx,0);
			grid->addWidget(line,gIdx,1);
			grid->addWidget(button, gIdx, 2);
			connect(button,SIGNAL(clicked()), this, SLOT(addFilter()));
		}
	}

}

MetaWidget::~MetaWidget()
{
	delete ui;
}

void MetaWidget::changeEvent(QEvent *e)
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

void MetaWidget::addFilter()
{
	if(QString(sender()->metaObject()->className()) == QString("QPushButton"))
	{
		QPushButton *b(reinterpret_cast<QPushButton*>(sender()));
		FMFontDb::InfoItem it(formFieldButton[b]);
		qDebug()<<"Meta:"<<FontStrings::Names()[it]<< formFieldLine[it]->text();
	}
}
