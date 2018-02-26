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
		currentPage(FAMILY_VIEW_INFO),
		currentIndex(0)
{
	ui->setupUi(this);


	ui->familyPreview->setNumCol(1);
	ui->familyPreview->setModelColumn(1);
	ui->familyPreview->setViewMode(QListView::IconMode);
	ui->familyPreview->setIconSize(QSize(ui->familyPreview->width(), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
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

void FamilyWidget::keyPressEvent(QKeyEvent *event)
{
	if(event->modifiers().testFlag(Qt::ControlModifier) && (event->key() == Qt::Key_PageUp))
	{
		switch(currentPage)
		{
		case FAMILY_VIEW_SAMPLE: slotShowActivation();
			break;
		case FAMILY_VIEW_CHART: slotShowSample();
			break;
		case FAMILY_VIEW_INFO: slotShowChart();
			break;
		case FAMILY_VIEW_ACTIVATION: slotShowInfo();
		default:break;
		}
	}
	else if(event->modifiers().testFlag(Qt::ControlModifier) && (event->key() == Qt::Key_PageDown))
	{
		switch(currentPage)
		{
		case FAMILY_VIEW_INFO: slotShowActivation();
			break;
		case FAMILY_VIEW_ACTIVATION: slotShowSample();
			break;
		case FAMILY_VIEW_SAMPLE: slotShowChart();
			break;
		case FAMILY_VIEW_CHART: slotShowInfo();
		default:break;
		}
	}
}

void FamilyWidget::slotPreviewUpdateSize(int w)
{
	ui->familyPreview->setIconSize(QSize(w, 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
}

void FamilyWidget::setFamily(const QString &f)
{
	if(f != family)
	{
		family = f;
		ui->familyLabel->setText(family);
		QList<FontItem*> fl( FMVariants::Order(FMFontDb::DB()->FamilySet(family)));
		ui->tagsWidget->prepare(fl);
		previewModel->resetBase(fl);
		if(!fl.isEmpty())
		{
			ui->familyPreview->setCurrentIndex( previewModel->index(0) );
			curVariant = fl.first()->path();
			emit fontSelected(curVariant);
		}
		delete sample;
		sample = 0;
		delete chart;
		chart = 0;
		delete activation;
		activation = 0;

		uniBlock = QString();
		slotShowSample();
	}
}

void FamilyWidget::slotPreviewUpdate()
{
	previewModel->dataChanged();
}

void FamilyWidget::slotPreviewSelected(const QModelIndex &index)
{
	QString fid(index.data(FMPreviewModel::PathRole).toString());
	if(fid != curVariant)
	{
		if(chart != 0)
			uniBlock = reinterpret_cast<ChartWidget*>(chart)->currentBlock();
		delete sample;
		delete chart;
		sample = chart = 0;
		curVariant = fid;
		currentIndex = index.row();
		switch(currentPage)
		{
		case FAMILY_VIEW_INFO: slotShowInfo();
			break;
		case FAMILY_VIEW_SAMPLE: slotShowSample();
			break;
		case FAMILY_VIEW_CHART: slotShowChart();
			break;
		default:
			break;
		}

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
			ui->displayStack->insertWidget(FAMILY_VIEW_SAMPLE, sw);
			sample = sw;
			connect(sample, SIGNAL(detached()), this, SLOT(slotDetachSample()));
		}
		ui->displayStack->setCurrentWidget(sample);
	}
	else
	{
		fw->show();
	}
	currentPage = FAMILY_VIEW_SAMPLE;
	updateButtons();
}

void FamilyWidget::slotShowInfo()
{
	FMInfoDisplay fid(FMFontDb::DB()->Font(curVariant));
	ui->webView->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
	ui->displayStack->setCurrentIndex(FAMILY_VIEW_INFO);
	currentPage = FAMILY_VIEW_INFO;
	updateButtons();
}

void FamilyWidget::slotShowChart()
{
	FloatingWidget * fw(FloatingWidgetsRegister::Widget(curVariant, ChartWidget::Name));
	if(fw == 0)
	{
		if(0 == chart)
		{
			ChartWidget *cw(new ChartWidget(curVariant, uniBlock, ui->pageChart));
			ui->displayStack->insertWidget(FAMILY_VIEW_CHART, cw);
			chart = cw;
			connect(chart, SIGNAL(detached()), this, SLOT(slotDetachChart()));
		}
		ui->displayStack->setCurrentWidget(chart);
	}
	else
	{
		fw->show();
	}
	currentPage = FAMILY_VIEW_CHART;
	updateButtons();
}

void FamilyWidget::slotShowActivation()
{
	FloatingWidget * fw(FloatingWidgetsRegister::Widget(curVariant, ActivationWidget::Name));
	if(fw == 0)
	{
		if(0 == activation)
		{
			ActivationWidget *aw(new ActivationWidget(family, ui->pageActivation));
			ui->displayStack->insertWidget(FAMILY_VIEW_ACTIVATION, aw);
			activation = aw;
			connect(activation, SIGNAL(familyStateChanged()), this, SLOT(slotStateChange()));
		}
		ui->displayStack->setCurrentWidget(activation);
	}
	else
	{
		fw->show();
	}
	currentPage = FAMILY_VIEW_ACTIVATION;
	updateButtons();
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
	uniBlock = reinterpret_cast<ChartWidget*>(chart)->currentBlock();
	chart = 0;
	slotShowInfo();
}

void FamilyWidget::slotStateChange()
{
	previewModel->resetBase(FMVariants::Order(FMFontDb::DB()->FamilySet(family)));
	emit familyStateChanged();
}

void FamilyWidget::updateButtons()
{
	static QList<QToolButton*> buttons;
	if(buttons.isEmpty())
	{
		buttons << ui->sampleButton
				<< ui->infoButton
				<< ui->chartButton
				<< ui->activationButton;
		foreach(QToolButton * b, buttons)
		{
			b->setCheckable(true);
		}
	}
	foreach(QToolButton * b, buttons)
	{
		b->setChecked(false);
	}
	switch(currentPage)
	{
	case FAMILY_VIEW_SAMPLE: ui->sampleButton->setChecked(true);
		break;
	case FAMILY_VIEW_ACTIVATION: ui->activationButton->setChecked(true);
		break;
	case FAMILY_VIEW_CHART: ui->chartButton->setChecked(true);
		break;
	case FAMILY_VIEW_INFO : ui->infoButton->setChecked(true);
		break;
	default:break;
	}
}

