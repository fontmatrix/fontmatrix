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
#include "floatingwidgetsregister.h"
#include "samplewidget.h"
#include "chartwidget.h"
#include "activationwidget.h"
#include "fmvariants.h"


#include <QColor>
#include <QListWidgetItem>
#include <QIcon>
#include <QDebug>


FamilyWidget::FamilyWidget(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::FamilyWidget),
		sample(0),
		chart(0),
		activation(0),
		forceReset(false)
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
	connect(ui->infoButton, SIGNAL(clicked()), this, SLOT(slotShowInfo()));
	connect(ui->sampleButton, SIGNAL(clicked()), this, SLOT(slotShowSample()));
	connect(ui->chartButton, SIGNAL(clicked()), this, SLOT(slotShowChart()));
	connect(ui->activationButton, SIGNAL(clicked()), this, SLOT(slotShowActivation()));
	connect(ui->tagsWidget, SIGNAL(tagAdded()), this, SIGNAL(tagAdded()));
	connect(ui->tagsWidget, SIGNAL(tagChanged()), this, SIGNAL(tagChanged()));

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

void FamilyWidget::setFamily(const QString &f, unsigned int curIdx )
{
	if((f != family) || forceReset)
	{
		family = f;
		ui->familyLabel->setText(family);
		QList<FontItem*> fl( FMVariants::Order(FMFontDb::DB()->FamilySet(family)));
		ui->tagsWidget->prepare(fl);
		previewModel->resetBase(fl);
		if(!fl.isEmpty())
		{
			ui->familyPreview->setCurrentIndex( previewModel->index(curIdx) );
			curVariant = fl.at(curIdx)->path();
			emit fontSelected(curVariant);
			if(!forceReset)
				slotShowInfo();
		}
		if((sample != 0) && !sample->isVisible())
		{
			delete sample;
			sample = 0;
		}
		if((chart != 0) && !chart->isVisible())
		{
			delete chart;
			chart = 0;
		}
		if((activation != 0) && !activation->isVisible())
		{
			delete activation;
			activation = 0;
		}

		forceReset = false;
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
	QString fid(index.data(FMPreviewModel::PathRole).toString());
	if(fid != curVariant)
	{
		slotShowInfo();
		delete sample;
		delete chart;
		sample = chart = 0;
		curVariant = fid;
		FontItem * fItem(FMFontDb::DB()->Font(curVariant));
		FMInfoDisplay fid(fItem);
		ui->webView->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
		emit fontSelected(curVariant);
	}
}


void FamilyWidget::slotShowSample()
{
	FloatingWidget * fw(FloatingWidgetsRegister::Widget(curVariant, SampleWidget::Name));
	if(fw == 0)
	{
		if(0 == sample)
		{
			SampleWidget *sw(new SampleWidget(curVariant, ui->pageSample));
			ui->displayStack->insertWidget(1, sw);
			sample = sw;
			connect(sample, SIGNAL(detached()), this, SLOT(slotDetachSample()));
		}
		ui->displayStack->setCurrentWidget(sample);
	}
	else
	{
		fw->show();
	}
}

void FamilyWidget::slotShowInfo()
{
	FMInfoDisplay fid(FMFontDb::DB()->Font(curVariant));
	ui->webView->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
	ui->displayStack->setCurrentIndex(0);
}

void FamilyWidget::slotShowChart()
{
	FloatingWidget * fw(FloatingWidgetsRegister::Widget(curVariant, ChartWidget::Name));
	if(fw == 0)
	{
		if(0 == chart)
		{
			ChartWidget *cw(new ChartWidget(curVariant, ui->pageChart));
			ui->displayStack->insertWidget(2, cw);
			chart = cw;
			connect(chart, SIGNAL(detached()), this, SLOT(slotDetachChart()));
		}
		ui->displayStack->setCurrentWidget(chart);
	}
	else
	{
		fw->show();
	}
}

void FamilyWidget::slotShowActivation()
{
	FloatingWidget * fw(FloatingWidgetsRegister::Widget(curVariant, ActivationWidget::Name));
	if(fw == 0)
	{
		if(0 == activation)
		{
			ActivationWidget *aw(new ActivationWidget(family, ui->pageActivation));
			ui->displayStack->insertWidget(3, aw);
			activation = aw;
			connect(activation, SIGNAL(familyStateChanged()), this, SLOT(slotStateChange()));
//			connect(activation, SIGNAL(detached()), this, SLOT(slotDetachActivation()));
		}
		ui->displayStack->setCurrentWidget(activation);
	}
	else
	{
		fw->show();
	}
}

void FamilyWidget::slotDetachSample()
{
	disconnect(sample, SIGNAL(detached()), this, SLOT(slotDetachSample()));
	sample = 0;
	slotShowInfo();
}

void FamilyWidget::slotDetachChart()
{
	disconnect(chart, SIGNAL(detached()), this, SLOT(slotDetachChart()));
	chart = 0;
	slotShowInfo();
}


//void FamilyWidget::slotDetachActivation()
//{
//	disconnect(activation, SIGNAL(detached()), this, SLOT(slotDetachActivation()));
//	activation = 0;
//	slotShowInfo();
//}

void FamilyWidget::slotStateChange()
{
	forceReset = true;
	setFamily(family);
	emit familyStateChanged();
}

