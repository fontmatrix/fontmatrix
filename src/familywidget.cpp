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
#include "fmactivate.h"
#include "fmactivationreport.h"

#include <QColor>
#include <QListWidgetItem>
#include <QIcon>
#include <QDebug>


FamilyWidget::FamilyWidget(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::FamilyWidget),
		sample(0),
		chart(0)
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

	ui->activateButton->setCheckable(true);
	ui->deactivateButton->setCheckable(true);

	// init sorted variant names
	variants.clear();
	QStringList weight;
	weight << QString("Hairline")
			<<	QString("Thin")
			<<	QString("UltraLight")
			<<	QString("ExtraLight")
			<<	QString("Light")
			<<	QString("Book")
			<<	QString("Normal")
			<<	QString("Regular")
			<<	QString("Roman")
			<<	QString("Plain")
			<<	QString("Medium")
			<<	QString()
			<<	QString("Demi")
			<<	QString("DemiBold")
			<<	QString("SemiBold")
			<<	QString("Bold")
			<<	QString("ExtraBold")
			<<	QString("Extra")
			<<	QString("Heavy")
			<<	QString("Black")
			<<	QString("ExtraBlack")
			<<	QString("UltraBlack")
			<<	QString("Ultra");

	QStringList slope;
	slope << QString()
			<< QString("Italic")
			<< QString("Oblique")
			<< QString("Slanted");

	QStringList width;
	width << QString()
			<< QString("UltraCompressed")
			<<	QString("Compressed")
			<<	QString("UltraCondensed")
			<<	QString("Condensed")
			<<	QString("SemiCondensed")
			<<	QString("Narrow")
			<<	QString("SemiExtended")
			<<	QString("SemiExpanded")
			<<	QString("Extended")
			<<	QString("Expanded")
			<<	QString("ExtraExtended")
			<<	QString("ExtraExpanded");

	QStringList  optical;
	optical << QString()
			<< QString("Poster")
			<<	QString("Display")
			<<	QString("SubHead")
			<<	QString("SmallText")
			<<	QString("Caption");

	foreach(const QString& w, weight)
	{
		foreach(const QString& s, slope)
		{
			foreach(const QString& wi, width)
			{
				foreach(const QString& o, optical)
				{
					appendVariants(w, s, wi, o);
				}
			}
		}
	}
//	foreach(const QStringList& v, variants)
//		qDebug()<<v.join(" ");

	connect(ui->returnListButton, SIGNAL(clicked()), this, SIGNAL(backToList()));
	connect(ui->familyPreview, SIGNAL(widthChanged(int)),this,SLOT(slotPreviewUpdateSize(int)));
	connect(ui->familyPreview,SIGNAL(activated ( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(ui->familyPreview,SIGNAL(clicked ( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(ui->familyPreview,SIGNAL(pressed( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(ui->infoButton, SIGNAL(clicked()), this, SLOT(slotShowInfo()));
	connect(ui->sampleButton, SIGNAL(clicked()), this, SLOT(slotShowSample()));
	connect(ui->chartButton, SIGNAL(clicked()), this, SLOT(slotShowChart()));
	connect(ui->tagsWidget, SIGNAL(tagAdded()), this, SIGNAL(tagAdded()));
	connect(ui->tagsWidget, SIGNAL(tagChanged()), this, SIGNAL(tagChanged()));

	connect(ui->activateButton, SIGNAL(clicked(bool)), this, SLOT(slotActivate(bool)));
	connect(ui->deactivateButton, SIGNAL(clicked(bool)), this, SLOT(slotDeactivate(bool)));

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

void FamilyWidget::appendVariants(const QString &w, const QString &s, const QString &wi, const QString &o)
{
	QStringList p;
	p << w << s << wi << o;
	QStringList l;
	foreach(const QString& s, p)
	{
		if(!s.isEmpty())
			l << s;
	}
	variants << l;
}

bool FamilyWidget::compareVariants(const QStringList &a, const QStringList &b)
{
	foreach(const QString& va, a)
	{
		if(!b.contains(va, Qt::CaseInsensitive))
			return false;
	}
	foreach(const QString& vb, b)
	{
		if(!a.contains(vb, Qt::CaseInsensitive))
			return false;
	}
	return true;
}

QList<FontItem*> FamilyWidget::orderVariants(QList<FontItem*> ul)
{
//	return ul;
	QList<FontItem*> ret;
	QMap<FontItem*, QStringList> fl;
	foreach(FontItem* f, ul)
	{
		fl.insert(f, f->variant().split(QString(" ")));
//		qDebug()<< f->variant()<< f->variant().split(QString(" "));
	}
	foreach(const QStringList& v, variants)
	{
		foreach(FontItem* f, fl.keys())
		{
			if(compareVariants(v,fl[f]))
			{
				ret.append(f);
				fl.remove(f);
			}
		}
	}
	if(fl.count() > 0)
	{
		// for Univers-like fonts, we get the number key
		QMap<int, QMap<QString,FontItem*> > ulikeFonts;
		bool intOK(false);
		foreach(FontItem* f, fl.keys())
		{
			intOK = false;
			QString fs(fl[f].first());
			int idx(fs.toInt(&intOK, 10));
			if(intOK)
			{
				ulikeFonts[idx][f->variant()] = f;
				fl.remove(f);
			}
		}
		foreach(int k, ulikeFonts.keys())
		{
			foreach(const QString& v, ulikeFonts[k].keys())
				ret << ulikeFonts[k][v];
		}

		// still fonts unsorted;
		if(fl.count() > 0)
		{
			QMap<QString, FontItem*> lastChance;
			foreach(FontItem* f, fl.keys())
			{
				lastChance[f->variant()] = f;
			}
			foreach(const QString& v, lastChance.keys())
				ret << lastChance[v];
		}
	}
	return ret;
}

void FamilyWidget::slotPreviewUpdateSize(int w)
{
	ui->familyPreview->setIconSize(QSize(qRound(w ), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
}

void FamilyWidget::setFamily(const QString &f, unsigned int curIdx )
{
	if(f != family)
	{
		family = f;
		ui->familyLabel->setText(family);
		QList<FontItem*> fl( orderVariants( FMFontDb::DB()->FamilySet(family)));
		ui->tagsWidget->prepare(fl);
		previewModel->resetBase(fl);
		if(!fl.isEmpty())
		{
			FMInfoDisplay fid(fl.at(curIdx));
			ui->webView->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
			ui->familyPreview->setCurrentIndex( previewModel->index(curIdx) );
			curVariant = fl.at(curIdx)->path();

			ui->activateButton->setChecked(fl.at(curIdx)->isActivated());
			ui->activateButton->setEnabled(!fl.at(curIdx)->isActivated());
			ui->deactivateButton->setChecked(!fl.at(curIdx)->isActivated());
			ui->deactivateButton->setEnabled(fl.at(curIdx)->isActivated());

			emit fontSelected(curVariant);
		}
		slotShowInfo();
		delete sample;
		delete chart;
		sample = chart = 0;
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

void FamilyWidget::slotActivate(bool c)
{
	if(c)
	{
		//		ui->activateButton->setChecked(true);
		//		ui->activateButton->setEnabled(false);
		//		ui->deactivateButton->setChecked(false);
		//		ui->deactivateButton->setEnabled(true);

		FMActivate::getInstance()->errors();
		FMActivate::getInstance()->activate(FMFontDb::DB()->FamilySet(family), true);
		QMap<QString,QString> actErr(FMActivate::getInstance()->errors());
		if(actErr.count() > 0)
		{
			FMActivationReport ar(this, actErr);
			ar.exec();
		}
		QString rf(family);
		family = QString();
		setFamily(rf, ui->familyPreview->currentIndex().row());
		emit familyStateChanged();
	}
}

void FamilyWidget::slotDeactivate(bool c)
{
	if(c)
	{
		//		ui->activateButton->setChecked(false);
		//		ui->activateButton->setEnabled(true);
		//		ui->deactivateButton->setChecked(true);
		//		ui->deactivateButton->setEnabled(false);

		FMActivate::getInstance()->errors();
		FMActivate::getInstance()->activate(FMFontDb::DB()->FamilySet(family), false);
		QMap<QString,QString> actErr(FMActivate::getInstance()->errors());
		if(actErr.count() > 0)
		{
			FMActivationReport ar(this, actErr);
			ar.exec();
		}
		QString rf(family);
		family = QString();
		setFamily(rf, ui->familyPreview->currentIndex().row());
		emit familyStateChanged();
	}
}

