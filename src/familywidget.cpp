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

#include "familywidget.h"
#include "ui_familywidget.h"
#include "typotek.h"
#include "fmpreviewlist.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fminfodisplay.h"
#include "samplewidget.h"

#include <QColor>
#include <QListWidgetItem>
#include <QIcon>


FamilyWidget::FamilyWidget(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::FamilyWidget)
{
	ui->setupUi(this);


	ui->familyPreview->setNumCol(1);
	ui->familyPreview->setModelColumn(1);
	ui->familyPreview->setViewMode(QListView::IconMode);
	ui->familyPreview->setIconSize(QSize(qRound(ui->familyPreview->width() ), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
	ui->familyPreview->setUniformItemSizes(true);
	ui->familyPreview->setMovement(QListView::Static);

	previewModel = new FMPreviewModel( this, ui->familyPreview );
	previewModel->setSpecString("<variant>");
	ui->familyPreview->setModel(previewModel);

	connect(ui->returnListButton, SIGNAL(clicked()), this, SIGNAL(backToList()));
	connect(ui->familyPreview, SIGNAL(widthChanged(int)),this,SLOT(slotPreviewUpdateSize(int)));
	connect(ui->familyPreview,SIGNAL(activated ( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(ui->familyPreview,SIGNAL(clicked ( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(ui->familyPreview,SIGNAL(pressed( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(ui->sampleButton, SIGNAL(clicked()), this, SLOT(slotShowSample()));

}

FamilyWidget::~FamilyWidget()
{
	delete ui;
}

TagsWidget* FamilyWidget::tagWidget()
{
	return ui->tagsWidget;
}

QWebView* FamilyWidget::info()
{
	return ui->webView;
}

void FamilyWidget::changeEvent(QEvent *e)
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

void FamilyWidget::slotPreviewUpdateSize(int w)
{
	ui->familyPreview->setIconSize(QSize(qRound(w ), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
}

void FamilyWidget::setFamily(const QString &family)
{
	ui->familyLabel->setText(family);
	QList<FontItem*> fl(FMFontDb::DB()->FamilySet(family));
	previewModel->resetBase(fl);
	if(!fl.isEmpty())
	{
		FMInfoDisplay fid(fl.first());
		ui->webView->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
		ui->familyPreview->setCurrentIndex( previewModel->index(0) );
		curVariant = fl.first()->path();
		emit fontSelected(curVariant);
	}
}

void FamilyWidget::slotPreviewUpdate()
{
	previewModel->dataChanged();
}

void FamilyWidget::slotPreviewSelected(const QModelIndex &index)
{
	//	FontItem * fItem(FMFontDb::DB()->Font(index.data(FMPreviewModel::PathRole)));
	//	if(!fItem)
	//	{
	//		qDebug()<<"\t-FontItme invalid";
	//		return;
	//	}
	//	QString fName(fItem->path());
	//	qDebug()<<"\t+"<< fName;
	//	if(!fName.isEmpty())
	//	{
	//		typotek::getInstance()->getTheMainView()->slotFontSelectedByName(index.data(FMPreviewModel::PathRole));
	//	}
	curVariant = index.data(FMPreviewModel::PathRole).toString();
	FontItem * fItem(FMFontDb::DB()->Font(curVariant));
	FMInfoDisplay fid(fItem);
	ui->webView->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
	emit fontSelected(curVariant);
}


void FamilyWidget::slotShowSample()
{
	SampleWidget* sw(new SampleWidget(curVariant));
	FontItem * fItem(FMFontDb::DB()->Font(curVariant));
	sw->setWindowTitle(QString("%1 - Fontmatrix").arg(fItem->fancyName()));
	sw->show();
}
