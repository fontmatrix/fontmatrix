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
#include "fmfontstrings.h"


#include <QStringListModel>
#include <QCompleter>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QComboBox>
#include <QVariant>

QStringListModel * MetaWidget::mModel = 0;
QStringList MetaWidget::mList = QStringList();

MetaWidget::MetaWidget(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::MetaWidget)
{
	ui->setupUi(this);
	if(mModel == 0)
	{
		mModel = new QStringListModel;
		mModel->setStringList(mList);
	}
//	QGridLayout * ui->grid(new QGridLayout(this));
	QCompleter * completer(new QCompleter(mModel));

	//	dont know why but it doesn't want to be placed in the ui->grid ###
//	QLabel *lab(new QLabel(tr("<div style=\"font-weight:bold;\">Fill-in a text field and press enter.</div>"), this));
//	ui->grid->addWidget(lab,0,0,0,-1);

	QList<FMFontDb::InfoItem> ln;
	ln << FMFontDb::FontFamily
			<< FMFontDb::FontSubfamily
			<< FMFontDb::Designer
			<< FMFontDb::Description
			<< FMFontDb::Copyright
			<< FMFontDb::Trademark
			<< FMFontDb::ManufacturerName
			<< FMFontDb::LicenseDescription
			<< FMFontDb::AllInfo;

	int limit = ((ln.count() + 1) / 2) - 1;

	for(int gIdx(0); gIdx < ln.count() ; ++gIdx)
	{
		FMFontDb::InfoItem k(ln[gIdx]);
//		if((k !=  FMFontDb::AllInfo))
		{
			QString fieldname(FontStrings::Names().value(k));
			if(k == FMFontDb::AllInfo)
				fieldname = QString("<div style=\"font-weight:bold\">%1</div>").arg(fieldname);
			QLabel *label(new QLabel(fieldname,this));
			QLineEdit *line(new QLineEdit(this));
			metFields[line] = k;
			line->setCompleter(completer);
			label->setBuddy(line);
			connect(line,SIGNAL(returnPressed()), this, SLOT(addFilter()));
			if((gIdx) < limit)
			{
				ui->grid->addWidget(label,gIdx,0);
				ui->grid->addWidget(line,gIdx ,1);
			}
			else
			{
				int row(gIdx - limit);
				ui->grid->addWidget(label, row, 3);
				ui->grid->addWidget(line, row , 4);
			}
		}
	}

	connect(ui->cancelButton, SIGNAL(clicked()), this, SIGNAL(Close()));
	connect(ui->filterButton, SIGNAL(clicked()), this, SLOT(addFilter()));
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
//	if(QString(sender()->metaObject()->className()) == QString("QLineEdit"))
//	{
//		QLineEdit *l(reinterpret_cast<QLineEdit*>(sender()));
//		FMFontDb::InfoItem it(metFields[l]);
//		QString t(l->text());
//		if(!mList.contains(t))
//		{
//			mList.append(t);
//			mModel->setStringList(mList);
//		}
//		resultMap[it] = t;
//	}
//	else
	{
		foreach(QLineEdit *l, metFields.keys())
		{
			QString t(l->text());
			FMFontDb::InfoItem it(metFields[l]);
			if(!t.isEmpty())
			{
				if(!mList.contains(t))
				{
					mList.append(t);
					mModel->setStringList(mList);
				}
				resultMap[it] = t;
			}
		}
	}
	emit filterAdded();
}
