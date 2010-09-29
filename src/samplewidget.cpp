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

#include "samplewidget.h"
#include "sampletoolbar.h"
#include "ui_samplewidget.h"
#include "typotek.h"
#include "fmbaseshaper.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fmlayout.h"
#include "textprogression.h"
#include "opentypetags.h"

#include <QApplication>
#include <QMap>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QTimer>
#include <QDataStream>
#include <QSettings>
#include <QStyledItemDelegate>
#include <QKeyEvent>


QByteArray SampleWidget::State::toByteArray() const
{
	QByteArray b;
	QDataStream ds(&b, QIODevice::WriteOnly);
	ds << sampleName;
	ds << fontSize;
	ds << renderHinting;
	ds << shaper;
	ds << script;
	return b;
}


void FMLayoutThread::setLayout(FMLayout * l, const QList<GlyphList>& spec , double fs, FontItem* f, unsigned int hinting)
{
	pLayout = l;
	pLayout->setContext(false);
	gl = spec;
	fontSize = fs;
	font = f;
	fHinting = hinting;
}

void FMLayoutThread::run()
{
	FontItem * tf(new FontItem(font->path(),font->family(),font->variant(),font->type(), font->isActivated()));
	tf->setFTHintMode(fHinting);
	pLayout->doLayout(gl, fontSize, tf);
	delete tf;
}

SampleWidget::State SampleWidget::State::fromByteArray(QByteArray b)
{
	QDataStream ds(&b, QIODevice::ReadOnly);
	QString sn;
	double fs;
	unsigned int rh;
	QString sh;
	QString sc;
	ds >> sn;
	ds >> fs;
	ds >> rh;
	ds >> sh;
	ds >> sc;
//	State*  pState(new State(sn,fs,rh,sh,sc));
//	State rState(*pState);
	sampleName = sn;
	fontSize = fs;
	renderHinting = rh;
	shaper = sh;
	script = sc;
//	return State(sn,fs,rh,sh,sc);
	return *this;
}

const QString SampleWidget::Name = QObject::tr("Sample");

SampleWidget::SampleWidget(const QString& fid, QWidget *parent) :
		FloatingWidget(fid, Name, parent),
		ui(new Ui::SampleWidget),
		fontIdentifier(fid)
{
	layoutTimer = new QTimer(this);
	layoutWait = 1000;
	layoutForPrint = false;
	layoutSwitch = false;
	ui->setupUi(this);
//	ui->textProgression->setVisible(false);

	sampleToolBar = new SampleToolBar(this);
	ui->sampleGridLayout->addWidget(sampleToolBar, 1,0, Qt::AlignRight | Qt::AlignBottom);


	sampleNameEditor = new QStyledItemDelegate(ui->sampleTextTree);
	ui->sampleTextTree->setItemDelegate(sampleNameEditor);
	refillSampleList();
	fillOTTree();

	sysWatcher = new QFileSystemWatcher(this);
	sysWatcher->addPath(fid);
	reloadTimer = new QTimer(this);
	reloadTimer->setInterval(1000);

	loremScene = new QGraphicsScene;
	ftScene =  new QGraphicsScene;
	QRectF pageRect ( 0,0,597.6,842.4 ); //TODO find means to smartly decide of page size (here, iso A4)

	loremScene->setSceneRect ( pageRect );

	ftScene->setSceneRect ( 0,0, 597.6 * typotek::getInstance()->getDpiX() / 72.0, 842.4 * typotek::getInstance()->getDpiX() / 72.0);
	ui->loremView->setScene ( loremScene );
	ui->loremView->locker = false;
	double horiScaleT (typotek::getInstance()->getDpiX() / 72.0);
	double vertScaleT ( typotek::getInstance()->getDpiY() / 72.0);
	QTransform adjustAbsoluteViewT( horiScaleT , 0, 0,vertScaleT, 0, 0 );
	ui->loremView->setTransform ( adjustAbsoluteViewT , false );

	ui->loremView_FT->setScene ( ftScene );
	ui->loremView_FT->locker = false;
	ui->loremView_FT->fakePage();

	layoutThread = new  FMLayoutThread;

	FontItem * cf(FMFontDb::DB()->Font(fid));
	textLayoutVect = new FMLayout(loremScene, cf);
	textLayoutFT =  new FMLayout(ftScene);


	QSettings settings;
	State s;
	s.fontSize = 14;
	QByteArray bs = settings.value("Sample/state", s.toByteArray()).toByteArray();
	setState(s.fromByteArray(bs));
	sampleRatio = 1.2;
	sampleInterSize = sampleFontSize * sampleRatio;

	createConnections();
	slotView();
}

SampleWidget::~SampleWidget()
{
	removeConnections();
	delete ui;
	delete loremScene;
	delete ftScene;
	delete textLayoutFT;
	delete textLayoutVect;
}

void SampleWidget::createConnections()
{
	// connections

	connect ( ui->loremView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateSView()));
	connect ( ui->loremView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));

	connect ( ui->loremView_FT, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
	connect ( ui->loremView_FT, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateRView()));

	connect ( textLayoutVect, SIGNAL(updateLayout()),this, SLOT(slotView()));
	connect ( this, SIGNAL(stopLayout()), textLayoutVect,SLOT(stopLayout()));
	connect ( textLayoutFT, SIGNAL(updateLayout()),this, SLOT(slotView()));
	connect ( this, SIGNAL(stopLayout()), textLayoutFT,SLOT(stopLayout()));

	connect ( ui->sampleTextTree,SIGNAL ( itemSelectionChanged ()),this,SLOT ( slotSampleChanged() ) );
	connect ( ui->sampleTextTree,SIGNAL ( itemSelectionChanged ()),this,SLOT ( slotEditSample() ) );
	connect ( sampleToolBar, SIGNAL( SizeChanged(double) ),this,SLOT(slotLiveFontSize(double)));

	connect ( ui->OpenTypeTree, SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ), this, SLOT ( slotFeatureChanged() ) );
	connect ( ui->saveDefOTFBut, SIGNAL(released()),this,SLOT(slotDefaultOTF()));
	connect ( ui->resetDefOTFBut, SIGNAL(released()),this,SLOT(slotResetOTF()));

//	connect ( ui->textProgression, SIGNAL ( stateChanged (  ) ),this ,SLOT(slotProgressionChanged()));

	connect(ui->toolbar, SIGNAL(Print()), this, SLOT(slotPrint()));
	connect(ui->toolbar, SIGNAL(Close()), this, SLOT(close()));
	connect(ui->toolbar, SIGNAL(Hide()), this, SLOT(hide()));
	connect(ui->toolbar, SIGNAL(Detach()), this, SLOT(ddetach()));

	connect(sysWatcher, SIGNAL(fileChanged(QString)),this, SLOT(slotFileChanged(QString)));
	connect(reloadTimer,SIGNAL(timeout()), this, SLOT(slotReload()));

	connect(this, SIGNAL(stateChanged()), this, SLOT(saveState()));

	connect(sampleToolBar, SIGNAL(OpenTypeToggled(bool)), this, SLOT(slotShowOpenType(bool)));
	connect(sampleToolBar, SIGNAL(SampleToggled(bool)), this, SLOT(slotShowSamples(bool)));
	connect(sampleToolBar, SIGNAL(ScriptSelected()), this, SLOT(slotScriptChange()));

	connect(ui->addSampleButton, SIGNAL(clicked()), this, SLOT(slotAddSample()));
	connect(ui->removeSampleButton, SIGNAL(clicked()), this, SLOT(slotRemoveSample()));
	connect(sampleNameEditor, SIGNAL(closeEditor(QWidget*)), this, SLOT(slotSampleNameEdited(QWidget*)));
	connect(ui->sampleEdit, SIGNAL(textChanged()), this, SLOT(slotUpdateSample()));


	connect(textLayoutFT, SIGNAL(clearScene()), this, SLOT(clearFTScene()));
}


void SampleWidget::removeConnections()
{

	disconnect ( ui->loremView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateSView()));
	disconnect ( ui->loremView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));

	disconnect ( ui->loremView_FT, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
	disconnect ( ui->loremView_FT, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateRView()));

	disconnect ( textLayoutVect, SIGNAL(updateLayout()),this, SLOT(slotView()));
	disconnect ( this, SIGNAL(stopLayout()), textLayoutVect,SLOT(stopLayout()));
	disconnect ( textLayoutFT, SIGNAL(updateLayout()),this, SLOT(slotView()));
	disconnect ( this, SIGNAL(stopLayout()), textLayoutFT,SLOT(stopLayout()));

	disconnect ( ui->sampleTextTree,SIGNAL ( itemSelectionChanged ()),this,SLOT ( slotSampleChanged() ) );
	disconnect ( sampleToolBar, SIGNAL( SizeChanged(double) ),this,SLOT(slotLiveFontSize(double)));

	disconnect ( ui->OpenTypeTree, SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ), this, SLOT ( slotFeatureChanged() ) );
	disconnect ( ui->saveDefOTFBut, SIGNAL(released()),this,SLOT(slotDefaultOTF()));
	disconnect ( ui->resetDefOTFBut, SIGNAL(released()),this,SLOT(slotResetOTF()));

//	disconnect ( ui->textProgression, SIGNAL ( stateChanged (  ) ),this ,SLOT(slotProgressionChanged()));

	disconnect(ui->toolbar, SIGNAL(Print()), this, SLOT(slotPrint()));
	disconnect(ui->toolbar, SIGNAL(Close()), this, SLOT(close()));
	disconnect(ui->toolbar, SIGNAL(Hide()), this, SLOT(hide()));
	disconnect(ui->toolbar, SIGNAL(Detach()), this, SLOT(ddetach()));

	disconnect(sysWatcher, SIGNAL(fileChanged(QString)),this, SLOT(slotFileChanged(QString)));

	disconnect(this, SIGNAL(stateChanged()), this, SLOT(saveState()));
}

void SampleWidget::changeEvent(QEvent *e)
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


QGraphicsScene * SampleWidget::textScene() const
{
	return loremScene;
}

SampleWidget::State SampleWidget::state() const
{
	State ret;
	ret.set = true;
	ret.fontSize = sampleToolBar->getFontSize();
//	ret.renderRaster = ui->freetypeRadio->isChecked();
	ret.renderHinting = 0;
//	if(ui->normalHinting->isChecked())
//		ret.renderHinting = 1;
//	else if(ui->lightHinting->isChecked())
//		ret.renderHinting = 2;
	ret.sampleName = ui->sampleTextTree->currentItem()->data(0, Qt::UserRole).toString();
	if(ui->useShaperCheck->isChecked())
	{
		ret.script = ui->langCombo->currentText();
		ret.shaper = ui->shaperTypeCombo->currentText();
	}
	return ret;
}

void SampleWidget::setState(const SampleWidget::State &s)
{
	if(!s.set)
		return;
	sampleToolBar->setFontSize( s.fontSize );
	reSize( s.fontSize, s.fontSize * sampleRatio );

//	{
//		switch(s.renderHinting)
//		{
//		case 0: ui->noHinting->setChecked(true);
//			break;
//		case 1: ui->normalHinting->setChecked(true);
//			break;
//		case 2: ui->lightHinting->setChecked(true);
//			break;
//		default:break;
//		}
//	}

	QTreeWidgetItem * targetItem = 0;
	for(int i(0); i < ui->sampleTextTree->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem * tli(ui->sampleTextTree->topLevelItem(i));
		for(int ii(0); ii < tli->childCount(); ++ii)
		{
			if(tli->child(ii)->data(0, Qt::UserRole).toString() == s.sampleName)
			{
				targetItem = tli->child(ii);
				typotek::getInstance()->namedSample(s.sampleName);
				break;
			}
		}
		if(targetItem != 0)
			break;
	}
//	qDebug()<<"TI"<<targetItem;
	if(targetItem != 0)
		ui->sampleTextTree->setCurrentItem(targetItem, 0, QItemSelectionModel::SelectCurrent);

//	if(!s.shaper.isEmpty())
//	{
//		ui->useShaperCheck->setChecked(true);
//		ui->shaperTypeCombo->setCurrentIndex(ui->shaperTypeCombo->findText(s.shaper));
//		ui->langCombo->setCurrentIndex(ui->langCombo->findText(s.script));
//	}

	slotView();
	slotEditSample();
}

void SampleWidget::slotView()
{
	qDebug()<<"SampleWidget::slotView "<< fontIdentifier;
//	disconnect(textLayoutFT, SIGNAL(drawBaselineForMe(double)), this, SLOT(drawBaseline(double)));
	QTime t;
	t.start();
	FontItem *f(FMFontDb::DB()->Font( fontIdentifier ));
	if ( !f )
		return;

	bool wantDeviceDependant = !layoutForPrint;
	if(wantDeviceDependant)
	{
		f->setFTHintMode(hinting());
	}

//	if(ui->textProgression->inLine() == TextProgression::INLINE_LTR )
		f->setProgression(PROGRESSION_LTR );
//	else if(ui->textProgression->inLine() == TextProgression::INLINE_RTL )
//		f->setProgression(PROGRESSION_RTL);
//	else if(ui->textProgression->inLine() == TextProgression::INLINE_TTB )
//		f->setProgression(PROGRESSION_TTB );
//	else if(ui->textProgression->inLine() == TextProgression::INLINE_BTT )
//		f->setProgression(PROGRESSION_BTT);

	f->setFTRaster ( wantDeviceDependant );

//	if ( ui->loremView->isVisible() || ui->loremView_FT->isVisible() || layoutForPrint)
	{
		if(!layoutForPrint && !textLayoutFT->isLayoutFinished())
		{
			connect(textLayoutFT, SIGNAL(layoutFinished()), this, SLOT(slotView()));
			textLayoutFT->stopLayout();
			qDebug()<<"\tLayout stopped";
			layoutSwitch = false;
			return;
		}
		else
		{
			disconnect(textLayoutFT, SIGNAL(layoutFinished()), this, SLOT(slotView()));
		}
//		else if(textLayoutVect->isRunning())
//			textLayoutVect->stopLayout();
//		else
		{
			qDebug()<<"\tStart layout";
			ui->loremView_FT->unSheduleUpdate();
			ui->loremView->unSheduleUpdate();
			FMLayout * textLayout;
			if(layoutForPrint)
				textLayout = textLayoutVect;
			else
				textLayout = textLayoutFT;

			bool processFeatures = f->isOpenType() &&  !deFillOTTree().isEmpty();
			QString script(sampleToolBar->getScript());
			bool processScript( !script.isEmpty() );
			textLayout->setDeviceIndy(!wantDeviceDependant);
			textLayout->setAdjustedSampleInter( sampleInterSize );

			double fSize(sampleFontSize);

			QList<GlyphList> list;
			QStringList stl( typotek::getInstance()->namedSample().split("\n"));
//			qDebug()<<"Sample:\n\t"<<stl.join("\n\t");
			if ( processScript )
			{
				for(int p(0);p<stl.count();++p)
				{
					list << f->glyphs( stl[p] , fSize, script );
				}
			}
			else if(processFeatures)
			{
				for(int p(0);p<stl.count();++p)
				{
					list << f->glyphs( stl[p] , fSize, deFillOTTree());
				}
			}
			else
			{
				for(int p(0);p<stl.count();++p)
					list << f->glyphs( stl[p] , fSize  );
			}
			if(!layoutForPrint)
			{
				layoutThread->setLayout(textLayout, list, fSize, f, hinting());
				connect(textLayout, SIGNAL(drawPixmapForMe(int,double,double,double)), this, SLOT(drawPixmap(int,double,double,double)));
//				connect(textLayoutFT, SIGNAL(drawBaselineForMe(double)), this, SLOT(drawBaseline(double)));
				connect(textLayout, SIGNAL(layoutFinished()), this, SLOT(endLayout()));
				layoutSwitch = true;
				pixmapDrawn = 0;
				layoutThread->start();
			}
			else
			{
				textLayout->doLayout(list, fSize);
			}
		}
	}

}

void SampleWidget::drawPixmap(int index, double fontsize, double x, double y)
{
//	qDebug()<<"SampleWidget::drawPixmap index:"<<index<< "Y:"<<y;
	if(index < 0)
		disconnect(textLayoutFT, SIGNAL(drawPixmapForMe(int,double,double,double)), this, SLOT(drawPixmap(int,double,double,double)));
	FontItem * f( FMFontDb::DB()->Font( fontIdentifier ) );
	if(!f)
		return;
	++pixmapDrawn;
	QGraphicsPixmapItem *glyph = f->itemFromGindexPix ( index , fontsize );
//	qDebug()<<"SampleWidget::drawPixmap index:"<<index<< y << glyph->data(GLYPH_DATA_BITMAPTOP).toDouble();
	ftScene->addItem ( glyph );
	glyph->setZValue ( 100.0 );
	glyph->setPos ( x,y );
//	QGraphicsLineItem * l = ftScene->addLine(x,y,x,y + glyph->data(GLYPH_DATA_BITMAPTOP).toDouble());
//	l->setData(GLYPH_DATA_GLYPH, 1);
//	glyph->pixmap().toImage().save(QString("/tmp/%1.png").arg(index));
}

void SampleWidget::drawBaseline(double y)
{
	QGraphicsLineItem * l = ftScene->addLine(0,y,ftScene->width(),y);
	l->setData(GLYPH_DATA_GLYPH, 1);
}

void SampleWidget::clearFTScene()
{
	qDebug()<<"SampleWidget::clearFTScene"<< layoutSwitch;
//	if(layoutSwitch)
//		return;
	foreach(QGraphicsItem* gi, ftScene->items())
	{
		if(gi->data(GLYPH_DATA_GLYPH).toInt() > 0)
			delete gi;
	}
}

void SampleWidget::endLayout()
{
	QPointF texttopLeft(ui->loremView_FT->mapFromScene(textLayoutFT->getRect().topLeft()));
	ui->loremView_FT->translate( 10 -texttopLeft.x(), 10 -texttopLeft.y());
	ui->loremView_FT->update();
//	qDebug()<<"Pixmaps:"<<pixmapDrawn;

}

void SampleWidget::fillOTTree()
{
	ui->OpenTypeTree->clear();
	ui->langCombo->clear();
	ui->langCombo->setEnabled ( false );
	ui->useShaperCheck->setCheckState ( Qt::Unchecked );
	ui->useShaperCheck->setEnabled ( false );
	typotek * typo(typotek::getInstance());
	QStringList scripts;
	FontItem * theVeryFont(FMFontDb::DB()->Font( fontIdentifier ));
	if ( theVeryFont && theVeryFont->isOpenType() )
	{
		FMOtf * otf = theVeryFont->takeOTFInstance();
		foreach ( QString table, otf->get_tables() )
		{
			otf->set_table ( table );
			QTreeWidgetItem *tab_item = new QTreeWidgetItem ( ui->OpenTypeTree,QStringList ( table ) );
			tab_item->setExpanded ( true );
			foreach ( QString script, otf->get_scripts() )
			{
				scripts << script;
				otf->set_script ( script );
				QTreeWidgetItem *script_item = new QTreeWidgetItem ( tab_item, QStringList ( script ) );
				script_item->setExpanded ( true );
				foreach ( QString lang, otf->get_langs() )
				{
					otf->set_lang ( lang );
					QTreeWidgetItem *lang_item = new QTreeWidgetItem ( script_item, QStringList ( lang ) );
					lang_item->setExpanded ( true );
					foreach ( QString feature, otf->get_features() )
					{
						QStringList f ( feature );
						f << OTTagMeans ( feature );
						QTreeWidgetItem *feature_item = new QTreeWidgetItem ( lang_item, f );
						feature_item->setCheckState ( 0, Qt::Unchecked );
						if(table == "GPOS")
						{
							if(typo->getDefaultOTFScript() == script && typo->getDefaultOTFLang() == lang && typo->getDefaultOTFGPOS().contains(feature) )
							{
								feature_item->setCheckState ( 0, Qt::Checked );
							}
						}
						else if(table == "GSUB")
						{
							if(typo->getDefaultOTFScript() == script && typo->getDefaultOTFLang() == lang && typo->getDefaultOTFGSUB().contains(feature) )
							{
								feature_item->setCheckState ( 0, Qt::Checked );
							}
						}
					}
				}
			}
		}
		ui->OpenTypeTree->resizeColumnToContents ( 0 ) ;
		theVeryFont->releaseOTFInstance ( otf );
	}
	scripts = scripts.toSet().toList();
	// 	scripts.removeAll ( "latn" );
//	if ( !scripts.isEmpty() )
	{
//		ui->langCombo->setEnabled ( true );
//		ui->useShaperCheck->setEnabled ( true );
//		ui->langCombo->addItems ( scripts );
		sampleToolBar->setScripts(scripts);
	}
}

OTFSet SampleWidget::deFillOTTree()
{
	// 	qDebug() << "MainViewWidget::deFillOTTree()";
	OTFSet ret;
	// 	qDebug() << ui->OpenTypeTree->topLevelItemCount();
	for ( int table_index = 0; table_index < ui->OpenTypeTree->topLevelItemCount(); ++table_index ) //tables
	{
		// 		qDebug() << "table_index = " << table_index;
		QTreeWidgetItem * table_item = ui->OpenTypeTree->topLevelItem ( table_index ) ;
		// 		qDebug() <<  table_item->text(0);
		for ( int script_index = 0; script_index < table_item->childCount();++script_index ) //scripts
		{
			QTreeWidgetItem * script_item = table_item->child ( script_index );
			// 			qDebug() << "\tscript_index = " <<  script_index << script_item->text(0);
			for ( int lang_index = 0; lang_index < script_item->childCount(); ++lang_index ) //langs
			{
				QTreeWidgetItem * lang_item = script_item->child ( lang_index );
				// 				qDebug() << "\t\tlang_index = "<< lang_index << lang_item->text(0);
				for ( int feature_index = 0; feature_index < lang_item->childCount(); ++feature_index ) //features
				{
					// 					qDebug() << lang_item->childCount() <<" / "<<  feature_index;
					QTreeWidgetItem * feature_item = lang_item->child ( feature_index );
					// 					qDebug() << "\t\t\tfeature_item -> "<< feature_item->text(0);
					if ( feature_item->checkState ( 0 ) == Qt::Checked )
					{
						if ( table_item->text ( 0 ) == "GPOS" )
						{
							ret.script = script_item->text ( 0 );
							ret.lang = lang_item->text ( 0 );
							ret.gpos_features.append ( feature_item->text ( 0 ) );
						}
						if ( table_item->text ( 0 ) == "GSUB" )
						{
							ret.script = script_item->text ( 0 );
							ret.lang = lang_item->text ( 0 );
							ret.gsub_features.append ( feature_item->text ( 0 ) );
						}
					}
				}
			}
		}
	}
	// 	qDebug() << "endOf";
	return ret;

}

//void SampleWidget::slotChangeViewPage(QAbstractButton* but)
//{
//	QString radioName( but->objectName() );

//	if(radioName == "freetypeRadio" )
//	{
//		ui->stackedViews->setCurrentIndex(VIEW_PAGE_FREETYPE);
//		ui->hintingSelect->setEnabled(true);
//	}
//	else if(radioName == "nativeRadio" )
//	{
//		ui->stackedViews->setCurrentIndex(VIEW_PAGE_ABSOLUTE);
//		ui->hintingSelect->setEnabled(false);
//	}

//	slotView(true);
//}

//void SampleWidget::slotHintChanged(int )
//{
//	slotView(true);
//	emit stateChanged();
//}


//void SampleWidget::slotChangeViewPageSetting ( bool ch )
//{
//	// 	qDebug() <<"MainViewWidget::slotChangeViewPageSetting("<<ch<<")";
//	QString butName ( sender()->objectName() );
//	if ( !ch )
//	{
//		toolPanelWidth = ui->splitter_2->sizes().at ( 1 ) ;
//		ui->stackedTools->hide();
//	}
//	else
//	{
//		ui->stackedTools->show();
//		if ( ui->splitter_2->sizes().at ( 1 ) == 0 )
//		{
//			QList<int> li;
//			li << ui->splitter_2->width() - toolPanelWidth << toolPanelWidth;
//			ui->splitter_2->setSizes ( li );
//		}
//	}

//	QMap<QString, QToolButton*> bmap;
//	QMap<QString, int> pmap;
//	bmap[ "settingsButton" ] = ui->settingsButton;
//	bmap[ "openTypeButton" ] = ui->openTypeButton;
//	bmap[ "sampleButton" ] = ui->sampleButton;
//	pmap[ "settingsButton" ] = VIEW_PAGE_SETTINGS;
//	pmap[ "openTypeButton" ] = VIEW_PAGE_OPENTYPE;
//	pmap[ "sampleButton" ] = VIEW_PAGE_SAMPLES;

//	foreach(QString pk, bmap.keys())
//	{
//		if(butName == pk)
//		{
//			ui->stackedTools->setCurrentIndex(pmap[pk]);
//		}
//		else
//		{
//			bmap[pk]->setChecked ( false );
//		}
//	}
//}


void SampleWidget::slotUpdateSView()
{
	if(ui->loremView->isVisible())
		slotView();
}


void SampleWidget::slotZoom ( int z )
{
	double delta =  1.0 + ( z/1000.0 ) ;
	QTransform trans;
	trans.scale ( delta,delta );

	QGraphicsView * concernedView;
	if ( ui->loremView_FT->isVisible() )
		concernedView = ui->loremView_FT;
	else if ( ui->loremView->isVisible() )
	{
		concernedView = ui->loremView;
		if ( delta == 1.0 )
		{
			double horiScaleT (typotek::getInstance()->getDpiX() / 72.0);
			double vertScaleT ( typotek::getInstance()->getDpiY() / 72.0);
			QTransform adjustAbsoluteViewT( horiScaleT , 0, 0,vertScaleT, 0, 0 );
			trans =  adjustAbsoluteViewT;
		}
	}
	concernedView->setTransform ( trans, ( z == 0 ) ? false : true );

}


void SampleWidget::slotUpdateRView()
{
	if(ui->loremView_FT->isVisible())
		slotView();
}

void SampleWidget::slotSampleChanged()
{
	typotek::getInstance()->namedSample( ui->sampleTextTree->currentItem()->data(0, Qt::UserRole).toString() );
	ui->removeSampleButton->setEnabled(ui->sampleTextTree->currentItem()->parent() == uRoot);
	slotView (  );
	emit stateChanged();
}



void SampleWidget::slotLiveFontSize(double fs)
{
//	double fs( sampleToolBar->getFontSize() );
	reSize(fs, fs * sampleRatio);
	slotView();
	emit stateChanged();
}

void SampleWidget::slotFeatureChanged()
{
	// 	OTFSet ret = deFillOTTree();
	slotView (  );
	emit stateChanged();
}

void SampleWidget::slotDefaultOTF()
{
	OTFSet ots(deFillOTTree());
	typotek* typo(typotek::getInstance());
	typo->setDefaultOTFScript(ots.script);
	typo->setDefaultOTFLang(ots.lang);
	typo->setDefaultOTFGPOS(ots.gpos_features);
	typo->setDefaultOTFGSUB(ots.gsub_features);
}

void SampleWidget::slotResetOTF()
{
	typotek* typo(typotek::getInstance());
	typo->setDefaultOTFScript(QString());
	typo->setDefaultOTFLang(QString());
	typo->setDefaultOTFGPOS(QStringList());
	typo->setDefaultOTFGSUB(QStringList());
}


void SampleWidget::slotChangeScript()
{
	if ( ui->useShaperCheck->checkState() == Qt::Checked )
	{
		slotView (  );
	}
	emit stateChanged();
}

void SampleWidget::slotProgressionChanged()
{
	slotView();
}

void SampleWidget::slotWantShape()
{
	slotView (  );
	emit stateChanged();
}


void SampleWidget::refillSampleList()
{
	ui->sampleTextTree->clear();

	QTreeWidgetItem * curIt = 0;
	QMap<QString, QList<QString> > sl = typotek::getInstance()->namedSamplesNames();
	QList<QString> ul( sl.take(QString("User")) );
	if(ul.count())
	{
		uRoot = new QTreeWidgetItem(ui->sampleTextTree);
		//: Identify root of user defined sample texts
		uRoot->setText(0, tr("User"));
		bool first(true);
		foreach(QString uk, ul)
		{
			if(first)
			{
				first = false;
				uRoot->setData(0, Qt::UserRole , QString("User::") + uk);
				curIt = uRoot;
			}
			QTreeWidgetItem * it = new QTreeWidgetItem();
			it->setText(0, uk);
			it->setData(0, Qt::UserRole , QString("User::") + uk);
			uRoot->addChild(it);
		}
	}
	foreach(QString k, sl.keys())
	{
		QTreeWidgetItem * kRoot = new QTreeWidgetItem(ui->sampleTextTree);
		kRoot->setText(0, k);
		bool first(true);
		foreach(QString n, sl[k])
		{
			if(first)
			{
				first = false;
				kRoot->setData(0, Qt::UserRole , k + QString("::") + n);
				if(!curIt)
					curIt = kRoot;
			}
			QTreeWidgetItem * it = new QTreeWidgetItem();
			it->setText(0, n);
			it->setData(0, Qt::UserRole, k + QString("::") + n);
			kRoot->addChild(it);
		}
	}

	ui->sampleTextTree->setCurrentItem(curIt);
}

unsigned int SampleWidget::hinting()
{
//	if(ui->lightHinting->isChecked())
//		return FT_LOAD_TARGET_LIGHT;
//	else if(ui->normalHinting->isChecked())
//		return FT_LOAD_TARGET_NORMAL;

	return FT_LOAD_NO_HINTING ;
}


void SampleWidget::slotPrint()
{
	FontItem * font(FMFontDb::DB()->Font( fontIdentifier ));
	if(!font)
		return;
	layoutForPrint = true;
	slotView();
//	if( textLayoutVect->isRunning() )
//	{
//		connect(textLayoutVect, SIGNAL(paintFinished()), this,SLOT(slotPrint()));
//		return;
//	}
//	else
//	{
//		disconnect(textLayoutVect, SIGNAL(paintFinished()), this,SLOT(slotPrint()));
//	}

	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog dialog(&thePrinter, this);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Sample") +" - " + font->fancyName() );

	if ( dialog.exec() != QDialog::Accepted )
		return;
	thePrinter.setFullPage ( true );
	QPainter aPainter ( &thePrinter );

	loremScene->render(&aPainter);
	layoutForPrint = false;

}

void SampleWidget::slotFileChanged(const QString &)
{
	if(reloadTimer->isActive())
		reloadTimer->start();
	else
	{
		reloadTimer->start();
	}
}

void SampleWidget::slotReload()
{
//	reloadTimer->stop();
	slotView();
}

void SampleWidget::slotScriptChange()
{
	slotView();
}

void SampleWidget::saveState()
{
	QSettings settings;
	State s(state());
	QByteArray bs(s.toByteArray());
	settings.setValue("Sample/state", bs);
}

void SampleWidget::slotShowSamples(bool b)
{
	if(b)
	{
		if(sampleToolBar->isChecked(SampleToolBar::OpenTypeButton))
		{
			ui->sampleGridLayout->removeWidget(ui->opentypeWidget);
			ui->opentypeWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_OPENTYPE));
			sampleToolBar->toggle(SampleToolBar::OpenTypeButton, false);
		}
		ui->sampleEditWidget->setAutoFillBackground(true);
		ui->sampleEditWidget->resize(ui->sampleGridLayout->geometry().width() / 2, ui->sampleGridLayout->geometry().height());
		ui->sampleGridLayout->addWidget(ui->sampleEditWidget, 0,0, Qt::AlignRight);
	}
	else
	{
		ui->sampleGridLayout->removeWidget(ui->sampleEditWidget);
		ui->sampleEditWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_SAMPLES));
	}
}

void SampleWidget::slotShowOpenType(bool b)
{
	if(b)
	{
		if(sampleToolBar->isChecked(SampleToolBar::SampleButton))
		{
			ui->sampleGridLayout->removeWidget(ui->sampleEditWidget);
			ui->sampleEditWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_SAMPLES));
			sampleToolBar->toggle(SampleToolBar::SampleButton, false);
		}
		ui->opentypeWidget->setAutoFillBackground(true);
		ui->opentypeWidget->resize(ui->sampleGridLayout->geometry().width() / 2, ui->sampleGridLayout->geometry().height());
		ui->sampleGridLayout->addWidget(ui->opentypeWidget, 0,0, Qt::AlignRight);
	}
	else
	{
		ui->sampleGridLayout->removeWidget(ui->opentypeWidget);
		ui->opentypeWidget->setParent(ui->stackedViews->widget(VIEW_PAGE_OPENTYPE));
	}
}

void SampleWidget::slotAddSample()
{
	QString nu( tr("New Sample") );
	newSampleName = new QTreeWidgetItem();
	newSampleName->setText(0,nu);
	newSampleName->setData(0, Qt::UserRole , QString("NEW_SAMPLE"));
	newSampleName->setFlags(newSampleName->flags() | Qt::ItemIsEditable);
	uRoot->addChild(newSampleName);
	ui->sampleTextTree->openPersistentEditor(newSampleName);
	if(!uRoot->isExpanded())
		uRoot->setExpanded(true);
	ui->sampleTextTree->setCurrentItem(newSampleName);
}

void SampleWidget::slotSampleNameEdited(QWidget *w)
{
	ui->sampleTextTree->closePersistentEditor(newSampleName);
	newSampleName->setData(0, Qt::UserRole , QString("User::") + newSampleName->text(0));
	typotek::getInstance()->changeSample(newSampleName->text(0), ui->sampleEdit->toPlainText() );
}

void SampleWidget::slotRemoveSample()
{
	QString name(ui->sampleTextTree->currentItem()->text(0));
	QTreeWidgetItem * currentItem = ui->sampleTextTree->currentItem();
	uRoot->removeChild(currentItem);
	typotek::getInstance()->removeNamedSample(name);
}

void SampleWidget::slotEditSample()
{
	disconnect(ui->sampleEdit, SIGNAL(textChanged()), this, SLOT(slotUpdateSample()));
	QTreeWidgetItem * currentItem = ui->sampleTextTree->currentItem();
	ui->sampleEdit->setPlainText(typotek::getInstance()->namedSample( currentItem->data(0, Qt::UserRole).toString() ));
	ui->sampleEdit->setReadOnly(currentItem->parent() != uRoot);
	connect(ui->sampleEdit, SIGNAL(textChanged()), this, SLOT(slotUpdateSample()));
}

void SampleWidget::slotUpdateSample()
{
	typotek::getInstance()->changeSample(ui->sampleTextTree->currentItem()->text(0), ui->sampleEdit->toPlainText());
	slotView();
}

