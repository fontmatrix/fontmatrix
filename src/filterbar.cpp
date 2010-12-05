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
#include "filtersdialogitem.h"
#include "filteritem.h"
#include "filtertag.h"
#include "filterpanose.h"
#include "filtermeta.h"
#include "fmpaths.h"
#include "filtersdialog.h"
#include "typotek.h"

#include <QDialog>
#include <QGridLayout>
#include <QStringList>
#include <QInputDialog>
#include <QDir>
#include <QFile>
#include <QAction>
#include <QDebug>
#include <QCompleter>

QString FilterBar::andOpString = FilterBar::tr("And");
QString FilterBar::notOpString = FilterBar::tr("Not");
QString FilterBar::orOpString = FilterBar::tr("Or");

TagListModel::TagListModel(QObject *parent)
	:QAbstractTableModel(parent),
	specialTagsCount(1)
{
//	ui->tagsCombo->clear();
//	//	tagsetIcon = QIcon(":/fontmatrix_tagseteditor.png");

//	ui->tagsCombo->addItem(tr("Tags"),"NO_KEY");
//	ui->tagsCombo->addItem(tr("All activated"),"ALL_ACTIVATED");

//	QStringList tl_tmp = FMFontDb::DB()->getTags();
////	qDebug()<< "T"<< tl_tmp.join("||");
//	tl_tmp.sort();
//	foreach(QString tag, tl_tmp )
//	{
//		if(!FMFontDb::DB()->Fonts(tag, FMFontDb::Tags ).isEmpty())
//			ui->tagsCombo->addItem(tag, "TAG");
//	}
}

int TagListModel::rowCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;
	return FMFontDb::DB()->getTags().count() + specialTagsCount;
}

int TagListModel::columnCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;
	return 1; // let's start simple
}

QVariant TagListModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid() && index.column() != 0)
		return QVariant();
	QStringList tl_tmp = FMFontDb::DB()->getTags();
	tl_tmp.sort();
	// specials
	tl_tmp.prepend(tr("All activated"));

	QString tag(tl_tmp.at(index.row()));
	if(role == Qt::DisplayRole)
		return tag;
	else if(role == TagType)
	{
		if(index.row() < specialTagsCount)
		{
			return QString("ALL_ACTIVATED");
		}
		return QString("TAG");
	}
	return QVariant();
}

bool TagListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!index.isValid())
		return false;
	if(value.toString().isEmpty())
		return false;
	QStringList tl_tmp = FMFontDb::DB()->getTags();
	tl_tmp.sort();
	if(value.toString() == tl_tmp.at(index.row() - specialTagsCount))
		return false;
	FMFontDb::DB()->editTag ( tl_tmp.at(index.row() - specialTagsCount), value.toString());
	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags TagListModel::flags(const QModelIndex &index) const
{
	if(index.row() > specialTagsCount - 1)
		return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void TagListModel::tagsDBChanged()
{
	emit dataChanged(index(0,0),index(FMFontDb::DB()->getTags().count() + specialTagsCount -1 ,columnCount() -1));
}

FilterBar::FilterBar(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::FilterBar)
{
	ui->setupUi(this);

	tagListModel = new TagListModel(this);
	ui->tagsView->setShowGrid(false);
	ui->tagsView->horizontalHeader()->setStretchLastSection(true);
	ui->tagsView->verticalHeader()->hide();
	ui->tagsView->horizontalHeader()->hide();
	ui->tagsView->setModel(tagListModel);

	metaFieldsMenu = new QMenu(tr("Fields"), this);
	QList<FMFontDb::InfoItem> ln;
	metaFieldKey = int(FMFontDb::AllInfo);
	ln << FMFontDb::AllInfo
			<< FMFontDb::FontFamily
			<< FMFontDb::FontSubfamily
			<< FMFontDb::Designer
			<< FMFontDb::Description
			<< FMFontDb::Copyright
			<< FMFontDb::Trademark
			<< FMFontDb::ManufacturerName
			<< FMFontDb::LicenseDescription;
	for(int gIdx(0); gIdx < ln.count() ; ++gIdx)
	{
		FMFontDb::InfoItem k(ln[gIdx]);
		{
			QString fieldname(FontStrings::Names().value(k));
			QAction * ma(new QAction(fieldname, metaFieldsMenu));
			ma->setData(int(k));
			metaFieldsMenu->addAction(ma);
		}
	}
	ui->metadataTool->setMenu(metaFieldsMenu);
	ui->metaFieldLabel->setText(FontStrings::Names().value(FMFontDb::AllInfo));
	mModel = new QStringListModel;
	mModel->setStringList(mList);
	ui->metadataLineEdit->setCompleter(new QCompleter(mModel));

	loadFilters();

	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(slotSaveFilter()));
	connect(ui->tagsView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotTagSelect(QModelIndex)));
	connect(ui->metadataLineEdit, SIGNAL(editingFinished()), this, SLOT(metaFilter()));
	connect(metaFieldsMenu, SIGNAL(triggered(QAction*)), this, SLOT(metaSelectField(QAction*)));
	connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(slotClearFilter()));
	connect(ui->panoseWidget, SIGNAL(filterChanged()), this, SLOT(slotPanoFilter()));
	connect(FMFontDb::DB(), SIGNAL(tagsChanged()), tagListModel, SLOT(tagsDBChanged()));

	connect(ui->tagsArrow, SIGNAL(openChanged(bool)), this, SLOT(slotToggleTags(bool)));
	connect(ui->metadataArrow, SIGNAL(openChanged(bool)), this, SLOT(slotToggleMeta(bool)));
	connect(ui->panoseArrow, SIGNAL(openChanged(bool)), this, SLOT(slotTogglePano(bool)));
	connect(ui->filtersArrow, SIGNAL(openChanged(bool)), this, SLOT(slotToggleFilter(bool)));

	ui->tagsArrow->changeOpen(false);
	ui->metadataArrow->changeOpen(false);
	ui->panoseArrow->changeOpen(false);
	ui->filtersArrow->changeOpen(false);
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



void FilterBar::metaFilter()
{
	if(ui->metadataLineEdit->text().isEmpty())
		return;

	FilterMeta *fm(new FilterMeta);

	if(!mList.contains(ui->metadataLineEdit->text()))
	{
		mList.append(ui->metadataLineEdit->text());
		mModel->setStringList(mList);
	}

	fm->setData(FilterData::Text, FontStrings::Names().value(static_cast<FMFontDb::InfoItem>(metaFieldKey)) + QString(" : ") + ui->metadataLineEdit->text());
	fm->setData(FilterMeta::Field, metaFieldKey);
	fm->setData(FilterMeta::Value, ui->metadataLineEdit->text());
	//				fm->setData(FilterData::Or, false);
	//				fm->setData(FilterData::And, true);
	addFilterItem(fm, false);

	ui->metadataLineEdit->clear();
	processFilters();

}

void FilterBar::processFilters()
{
	if(filters.count() > 0)
	{
		FMFontDb::DB()->clearFilteredFonts();
		bool first(true);
		foreach(FilterItem* d, filters)
		{
			if(first)
			{
				d->hideOperation(FilterItem::AND);
				first = false;
			}
			d->filter()->operate();
		}
		emit filterChanged();
	}
}

void FilterBar::slotRemoveFilterItem(bool process)
{
	FilterItem * fi(reinterpret_cast<FilterItem*>(sender()));
	if(fi != 0)
	{
		filters.removeAll(fi);
		if(filters.count() == 0)
		{
			FMFontDb::DB()->filterAllFonts();
			emit filterChanged();
		}
		fi->deleteLater();
		if(process && (filters.count() > 0) )
			processFilters();
	}
}

void FilterBar::removeAllFilters()
{
	FMFontDb::DB()->filterAllFonts();
	foreach(FilterItem* d, filters)
	{
		d->deleteLater();
	}
	filters.clear();
}

void FilterBar::addFilterItem(FilterData *f, bool process)
{
	if(f != 0)
	{
		FilterItem * it(f->item());
		if(!filters.contains(it))
		{
			if(f->data(FilterData::Replace).toBool())
				removeAllFilters();
			connect(it, SIGNAL(remove()), this, SLOT(slotRemoveFilterItem()));
			connect(f, SIGNAL(Changed()), this, SLOT(processFilters()));
			filters.append(it);
			ui->filterListLayout->addWidget(it);

			if(process)
				processFilters();
		}
	}
}

QString FilterBar::filterString(FilterData *d, bool first)
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

void FilterBar::loadFilters()
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
		ui->filtersLayout->addWidget(fdi, 0, Qt::AlignTop);
		connect(fdi, SIGNAL(Filter(QString)), this, SLOT(slotLoadFilter(QString)));
		connect(fdi, SIGNAL(Remove(QString)), this, SLOT(slotRemoveFilter(QString)));
	}
}

void FilterBar::slotTagSelect(const QModelIndex & index)
{
	QString tag(tagListModel->data(index).toString());
	QString key(tagListModel->data(index,TagListModel::TagType).toString());

//	int selCount(ui->tagsView->selectionModel()->selectedIndexes().count());
//	if(selCount == 1)
	{
		foreach(FilterItem* f, filters)
		{
			if(f->filter()->data(FilterTag::Tag).toString() == tag)
				return;
		}
	}


	FilterTag * ft(new FilterTag);
	ft->setData(FilterData::Text, tag);
	ft->setData(FilterTag::Key, key);
	ft->setData(FilterTag::Tag, tag);
//	if( selCount > 1 )
//	{
//		ft->setData(FilterData::And, true);
//		ft->setData(FilterData::Or, false);
//	}
	addFilterItem(ft);
}

void FilterBar::slotPanoFilter()
{
	QMap<int,QList<int> > pv(ui->panoseWidget->getFilter());
	const QMap< FontStrings::PanoseKey, QMap<int, QString> >& ps(FontStrings::Panose());
	foreach(int k, pv.keys())
	{
		foreach(int v, pv[k])
		{
			FontStrings::PanoseKey pk (static_cast<FontStrings::PanoseKey>(k));
			QString text(FontStrings::PanoseKeyName(pk) + QString(" : ") + ps.value(pk).value(v));
			FilterPanose *fp(new FilterPanose);
			fp->setData(FilterData::Text, text);
			fp->setData(FilterPanose::Param, k);
			fp->setData(FilterPanose::Value, v);
			addFilterItem(fp);
		}
	}

}

void FilterBar::metaSelectField(QAction *action)
{
	metaFieldKey = action->data().toInt();
	ui->metaFieldLabel->setText(FontStrings::Names().value(FMFontDb::InfoItem(metaFieldKey)));
}

void FilterBar::slotClearFilter()
{
	removeAllFilters();
	emit filterChanged();
}

void FilterBar::slotSaveFilter()
{
	if(filters.isEmpty() /*|| fname.isEmpty()*/)
		return;

	bool ok;
	QString fname = QInputDialog::getText(this, tr("Fontmatrix - Filter name"),
					     tr("Filter name:"), QLineEdit::Normal,
					     QString(""), &ok);
	if (!ok || fname.isEmpty())
		return;

	QDir fdir(FMPaths::FiltersDir());
	if(!fdir.exists(fname))
		fdir.mkdir(fname);
	fdir.cd(fname);
	for(int i(0); i < filters.count(); ++i)
	{
		QFile f(fdir.absoluteFilePath( QString("%1-%2").arg(i, 3, 10, QChar('0')).arg(filters[i]->filter()->type()) ));
		if(f.open(QIODevice::WriteOnly))
		{
			f.write(filters[i]->filter()->toByteArray());
			f.close();
		}
	}
	loadFilters();
}

void FilterBar::slotLoadFilter(const QString &fname)
{
	removeAllFilters();
	QDir fdir(FMPaths::FiltersDir() + fname);
	QStringList flist(fdir.entryList(QDir::NoDotAndDotDot|QDir::Files, QDir::Name));
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
				addFilterItem(f);
			}
		}
	}
	processFilters();
}

void FilterBar::slotRemoveFilter(const QString &fname)
{

	if(fname.isEmpty())
		return;

	QDir fdir(FMPaths::FiltersDir());
	if(fdir.exists(fname))
	{
		fdir.cd(fname);
		QStringList flist(fdir.entryList(QDir::NoDotAndDotDot|QDir::Files));
		foreach(QString fn, flist)
		{
			fdir.remove(fn);
		}
		fdir.cd(FMPaths::FiltersDir());
		fdir.rmdir(fname);
	}
	else
	{
		qDebug()<< "Directory does not exist:"<<fdir.absolutePath()<<fname;
	}
	loadFilters();
}

void FilterBar::filtersDialog()
{
	FiltersDialog *fd(new FiltersDialog(filters, this));
	connect(fd, SIGNAL(Filter(QString)), this, SLOT(slotLoadFilter(QString)));
	connect(fd, SIGNAL(AddFilter(QString)), this, SLOT(slotSaveFilter(QString)));
	connect(fd, SIGNAL(RemoveFilter(QString)), this, SLOT(slotRemoveFilter(QString)));
	fd->exec();
}

void FilterBar::slotToggleTags(bool t)
{
	if(t)
		ui->tagsBox->show();
	else
		ui->tagsBox->hide();
}

void FilterBar::slotToggleMeta(bool t)
{
	if(t)
		ui->metadataBox->show();
	else
		ui->metadataBox->hide();
}

void FilterBar::slotTogglePano(bool t)
{
	if(t)
		ui->panoseBox->show();
	else
		ui->panoseBox->hide();
}

void FilterBar::slotToggleFilter(bool t)
{
	if(t)
		ui->filtersBox->show();
	else
		ui->filtersBox->hide();
}
