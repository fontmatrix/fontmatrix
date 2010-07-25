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

#include "filtersdialog.h"
#include "ui_filtersdialog.h"

#include "filtersdialogitem.h"
#include "filtertag.h"
#include "filterpanose.h"
#include "filtermeta.h"
#include "fmpaths.h"
#include "filteritem.h"

#include <QDir>
#include <QFile>

QString FiltersDialog::andOpString = FiltersDialog::tr("And");
QString FiltersDialog::notOpString = FiltersDialog::tr("Not");
QString FiltersDialog::orOpString = FiltersDialog::tr("Or");


FiltersDialog::FiltersDialog(const QList<FilterItem*>& currentFilters, QWidget *parent) :
		QDialog(parent),
		ui(new Ui::FiltersDialog)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);
	ui->frame->setVisible(false);

	loadFilters();

	connect(ui->addButton, SIGNAL(clicked()), this, SLOT(slotAddFilter()));
	connect(ui->moreButton, SIGNAL(clicked(bool)), this, SLOT(showAdd(bool)));

	if(!currentFilters.isEmpty())
	{
		QString fs;
		bool first(true);
		foreach(FilterItem *f, currentFilters)
		{
			FilterData *d(f->filter());
			if(first)
			{
				first = false;
				fs += filterString(d, true);
			}
			else
				fs += filterString(d);

		}
		ui->curFilter->setText(fs);
	}
	else
	{
		ui->moreButton->setEnabled(false);
	}
}

FiltersDialog::~FiltersDialog()
{
	delete ui;
}


QString FiltersDialog::filterString(FilterData *d, bool first)
{
	QString fs;
	if(first)
	{
		first = false;
		if(d->data(FilterData::Not).toBool())
			fs += notOpString + QString(" [%1] ").arg(d->getText());
		else
			fs += QString("[%1] ").arg(d->getText());
	}
	else
	{
		if(d->data(FilterData::Or).toBool())
			fs += orOpString;
		else
			fs += andOpString;

		if(d->data(FilterData::Not).toBool())
			fs += QString(" %1").arg(notOpString);
		fs += QString(" [%1] ").arg(d->getText());
	}
	return fs;
}

void FiltersDialog::loadFilters()
{
	foreach(FiltersDialogItem* i, items)
		delete i;
	items.clear();

	QDir fbasedir(FMPaths::FiltersDir());
	QStringList fbaselist(fbasedir.entryList(QDir::NoDotAndDotDot|QDir::Dirs,QDir::Name));
	foreach(QString fname, fbaselist)
	{
		QDir fdir(FMPaths::FiltersDir() + fname);
		QStringList flist(fdir.entryList(QDir::NoDotAndDotDot|QDir::Files, QDir::Name));
		QString fString;
		bool first(true);
		foreach(QString fn, flist)
		{
			QStringList l(fn.split(QString("-")));
			if(l.count() == 2)
			{
				QString type(l.at(1));
				QFile file(fdir.absoluteFilePath(fn));
				if(file.open(QIODevice::ReadOnly))
				{
					FilterData *f;
					if(type == QString("Meta"))
					{
						f = new FilterMeta;
					}
					else if(type == QString("Panose"))
					{
						f = new FilterPanose;
					}
					else if(type == QString("Tag"))
					{
						f = new FilterTag;
					}
					f->fromByteArray(file.readAll());
					if(first)
					{
						first = false;
						fString += filterString(f, true);
					}
					else
						fString += filterString(f);
					delete f;
				}
			}
		}
		FiltersDialogItem *fdi(new FiltersDialogItem(fname, fString, this));
		items.append(fdi);
		ui->filtersLayout->addWidget(fdi);
		connect(fdi, SIGNAL(Filter(QString)), this, SIGNAL(Filter(QString)));
		connect(fdi, SIGNAL(Remove(QString)), this, SIGNAL(RemoveFilter(QString)));
	}
}


void FiltersDialog::slotAddFilter()
{
	QString fname(ui->newName->text());
	if(!fname.isEmpty())
		emit AddFilter(fname);

	loadFilters();
	showAdd(false);
	ui->moreButton->setChecked(false);
}

void FiltersDialog::showAdd(bool v)
{
	ui->frame->setVisible(v);
}
