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
#include "ui_samplewidget.h"
#include "typotek.h"
#include "fmbaseshaper.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fmlayout.h"
#include "textprogression.h"
#include "opentypetags.h"

#include <QMap>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QPrintDialog>
#include <QPrinter>

SampleWidget::SampleWidget(const QString& fid, QWidget *parent) :
		QWidget(parent),
		ui(new Ui::SampleWidget),
		fontIdentifier(fid)
{
	layoutForPrint = false;
	ui->setupUi(this);
	refillSampleList();
	fillOTTree();

	radioRenderGroup = new QButtonGroup();
	radioRenderGroup->addButton(ui->freetypeRadio);
	radioRenderGroup->addButton(ui->nativeRadio);
	ui->stackedTools->setCurrentIndex(VIEW_PAGE_SAMPLES);
	toolPanelWidth = ui->splitter_2->sizes().at(1);
	if(toolPanelWidth == 0)
	{
		ui->sampleButton->setChecked(false);
		ui->stackedTools->hide();
		toolPanelWidth = ui->splitter_2->width()/3;
	}
	radioFTHintingGroup = new QButtonGroup(ui->freetypeRadio);
	radioFTHintingGroup->addButton(ui->noHinting);
	radioFTHintingGroup->addButton(ui->lightHinting);
	radioFTHintingGroup->addButton(ui->normalHinting);

	loremScene = new QGraphicsScene;
	ftScene =  new QGraphicsScene;
	QRectF pageRect ( 0,0,597.6,842.4 ); //TODO find means to smartly decide of page size (here, iso A4)

	loremScene->setSceneRect ( pageRect );
	// 	QGraphicsRectItem *backp = loremScene->addRect ( pageRect,QPen(),Qt::white );
	// 	backp->setEnabled ( false );

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

	QMap<QString, int> sTypes(FMShaperFactory::types());
	for(QMap<QString, int>::iterator sIt = sTypes.begin(); sIt != sTypes.end() ; ++sIt)
	{
		ui->shaperTypeCombo->addItem(sIt.key(), sIt.value());
	}

	QSettings settings;
	sampleFontSize = settings.value("Sample/FontSize", 14.0).toDouble();
	sampleInterSize = settings.value("Sample/Interline", 18.0).toDouble();
	sampleRatio = sampleInterSize / sampleFontSize  ;
	ui->liveFontSizeSpin->setValue(sampleFontSize);


	FontItem * cf(FMFontDb::DB()->Font(fid));
	textLayoutVect = new FMLayout(loremScene, cf);
	textLayoutFT =  new FMLayout(ftScene, cf);

	// connections
	connect (radioRenderGroup,SIGNAL(buttonClicked( QAbstractButton* )),this,SLOT(slotChangeViewPage(QAbstractButton*)));
	connect (radioFTHintingGroup, SIGNAL(buttonClicked(int)),this,SLOT(slotHintChanged(int)));

	connect (ui->openTypeButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	connect (ui->settingsButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	connect (ui->sampleButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));

	connect ( ui->loremView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateSView()));
	connect ( ui->loremView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));

	connect ( ui->loremView_FT, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
	connect ( ui->loremView_FT, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateRView()));

	connect ( textLayoutVect, SIGNAL(updateLayout()),this, SLOT(slotView()));
	connect ( this, SIGNAL(stopLayout()), textLayoutVect,SLOT(stopLayout()));
	connect ( textLayoutFT, SIGNAL(updateLayout()),this, SLOT(slotView()));
	connect ( this, SIGNAL(stopLayout()), textLayoutFT,SLOT(stopLayout()));

	connect ( ui->sampleTextTree,SIGNAL ( itemSelectionChanged ()),this,SLOT ( slotSampleChanged() ) );
	connect ( ui->sampleTextButton, SIGNAL(released()),this, SLOT(slotEditSampleText()));
	connect ( ui->liveFontSizeSpin, SIGNAL( editingFinished() ),this,SLOT(slotLiveFontSize()));

	connect ( ui->OpenTypeTree, SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ), this, SLOT ( slotFeatureChanged() ) );
	connect ( ui->saveDefOTFBut, SIGNAL(released()),this,SLOT(slotDefaultOTF()));
	connect ( ui->resetDefOTFBut, SIGNAL(released()),this,SLOT(slotResetOTF()));
	connect ( ui->shaperTypeCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotChangeScript() ) );
	connect ( ui->langCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotChangeScript() ) );

	connect ( ui->textProgression, SIGNAL ( stateChanged (  ) ),this ,SLOT(slotProgressionChanged()));
	connect ( ui->useShaperCheck,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotWantShape() ) );

	connect(ui->printButton, SIGNAL(clicked()), this, SLOT(slotPrint()));
	connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));

	slotView(true);
}

SampleWidget::~SampleWidget()
{
	delete ui;
	delete loremScene;
	delete ftScene;
	delete textLayoutFT;
	delete textLayoutVect;
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

void SampleWidget::slotView ( bool needDeRendering )
{
	QTime t;
	t.start();
	FontItem *f(FMFontDb::DB()->Font( fontIdentifier ));
	if ( !f )
		return;
	if ( needDeRendering )
	{
		f->deRenderAll();
	}

	bool wantDeviceDependant = (!layoutForPrint) ? ui->loremView_FT->isVisible() : false;
	//	unsigned int storedHinting(f->getFTHintMode());
	if(wantDeviceDependant)
	{
		f->setFTHintMode(hinting());
	}

	if(ui->textProgression->inLine() == TextProgression::INLINE_LTR )
		f->setProgression(PROGRESSION_LTR );
	else if(ui->textProgression->inLine() == TextProgression::INLINE_RTL )
		f->setProgression(PROGRESSION_RTL);
	else if(ui->textProgression->inLine() == TextProgression::INLINE_TTB )
		f->setProgression(PROGRESSION_TTB );
	else if(ui->textProgression->inLine() == TextProgression::INLINE_BTT )
		f->setProgression(PROGRESSION_BTT);

	f->setFTRaster ( wantDeviceDependant );
	f->setShaperType(ui->shaperTypeCombo->itemData( ui->shaperTypeCombo->currentIndex() ).toInt() );

	if ( ui->loremView->isVisible() || ui->loremView_FT->isVisible() || layoutForPrint)
	{
		//		qDebug()<<"lv(ft) is visible";
		if(textLayoutFT->isRunning())
			textLayoutFT->stopLayout();
		else if(textLayoutVect->isRunning())
			textLayoutVect->stopLayout();
		else
		{
			//			qDebug()<<"tl is NOT running";
			QGraphicsScene *targetScene;
			ui->loremView_FT->unSheduleUpdate();
			ui->loremView->unSheduleUpdate();
			FMLayout * textLayout;
			if(ui->loremView->isVisible() || layoutForPrint)
			{
				textLayout = textLayoutVect;
			}
			else if(ui->loremView_FT->isVisible())
			{
				textLayout = textLayoutFT;
			}

			bool processFeatures = f->isOpenType() &&  !deFillOTTree().isEmpty();
			QString script = ui->langCombo->currentText();
			bool processScript =  f->isOpenType() && ( ui->useShaperCheck->checkState() == Qt::Checked ) && ( !script.isEmpty() );
			textLayout->setDeviceIndy(!wantDeviceDependant);
			textLayout->setAdjustedSampleInter( sampleInterSize );

			double fSize(sampleFontSize);

			QList<GlyphList> list;
			QStringList stl( typotek::getInstance()->namedSample(ui->sampleTextTree->currentItem()->data(0, Qt::UserRole).toString() ).split("\n"));
			if ( processScript )
			{
				for(int p(0);p<stl.count();++p)
				{
					list << f->glyphs( stl[p] , fSize, script );
				}
			}
			else if(processFeatures)
			{
				// Experimental code to handle alternate is commented out
				// Do not uncomment
				//				FMAltContext * actx ( FMAltContextLib::SetCurrentContext(sampleTextTree->currentText(), f->path()));
				//				int rs(0);
				//				actx->setPar(rs);
				for(int p(0);p<stl.count();++p)
				{
					list << f->glyphs( stl[p] , fSize, deFillOTTree());
					//					actx->setPar(++rs);
				}
				//				actx->cleanup();
				//				FMAltContextLib::SetCurrentContext(sampleTextTree->currentText(), f->path());
			}
			else
			{
				for(int p(0);p<stl.count();++p)
					list << f->glyphs( stl[p] , fSize  );
			}
			textLayout->doLayout(list, fSize);
			// 			if (loremView->isVisible() /*&& fitViewCheck->isChecked()*/ )
			// 			{
			// 				loremView->fitInView ( textLayout->getRect(), Qt::KeepAspectRatio );
			// 			}
			if(!layoutForPrint)
				textLayout->start(QThread::LowestPriority);
			else
				textLayout->run();
		}
	}
	else if(!ui->loremView->isVisible() && !ui->loremView_FT->isVisible())
	{
		ui->loremView->sheduleUpdate();
		ui->loremView_FT->sheduleUpdate();
	}

	//	slotUpdateGView();
	//	slotInfoFont();

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
	if ( !scripts.isEmpty() )
	{
		ui->langCombo->setEnabled ( true );
		ui->useShaperCheck->setEnabled ( true );
		ui->langCombo->addItems ( scripts );
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

void SampleWidget::slotChangeViewPage(QAbstractButton* but)
{
	QString radioName( but->objectName() );

	if(radioName == "freetypeRadio" )
	{
		ui->stackedViews->setCurrentIndex(VIEW_PAGE_FREETYPE);
		ui->hintingSelect->setEnabled(true);
	}
	else if(radioName == "nativeRadio" )
	{
		ui->stackedViews->setCurrentIndex(VIEW_PAGE_ABSOLUTE);
		ui->hintingSelect->setEnabled(false);
	}

	slotView(true);
}

void SampleWidget::slotHintChanged(int )
{
	slotView(true);
}


void SampleWidget::slotChangeViewPageSetting ( bool ch )
{
	// 	qDebug() <<"MainViewWidget::slotChangeViewPageSetting("<<ch<<")";
	QString butName ( sender()->objectName() );
	if ( !ch )
	{
		toolPanelWidth = ui->splitter_2->sizes().at ( 1 ) ;
		ui->stackedTools->hide();
	}
	else
	{
		ui->stackedTools->show();
		if ( ui->splitter_2->sizes().at ( 1 ) == 0 )
		{
			QList<int> li;
			li << ui->splitter_2->width() - toolPanelWidth << toolPanelWidth;
			ui->splitter_2->setSizes ( li );
		}
	}

	QMap<QString, QToolButton*> bmap;
	QMap<QString, int> pmap;
	bmap[ "settingsButton" ] = ui->settingsButton;
	bmap[ "openTypeButton" ] = ui->openTypeButton;
	bmap[ "sampleButton" ] = ui->sampleButton;
	pmap[ "settingsButton" ] = VIEW_PAGE_SETTINGS;
	pmap[ "openTypeButton" ] = VIEW_PAGE_OPENTYPE;
	pmap[ "sampleButton" ] = VIEW_PAGE_SAMPLES;

	foreach(QString pk, bmap.keys())
	{
		if(butName == pk)
		{
			ui->stackedTools->setCurrentIndex(pmap[pk]);
		}
		else
		{
			bmap[pk]->setChecked ( false );
		}
	}
}


void SampleWidget::slotUpdateSView()
{
	if(ui->loremView->isVisible())
		slotView(true);
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
		slotView(true);
}

void SampleWidget::slotSampleChanged()
{
	slotView ( true );
}

void SampleWidget::slotEditSampleText()
{
	typotek::getInstance()->slotPrefsPanel(PrefsPanelDialog::PAGE_SAMPLETEXT);
}

void SampleWidget::slotLiveFontSize()
{
	double fs( ui->liveFontSizeSpin->value() );
	reSize(fs, fs * sampleRatio);
	slotView(true);
}

void SampleWidget::slotFeatureChanged()
{
	// 	OTFSet ret = deFillOTTree();
	slotView ( true );
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
		slotView ( true );
	}
}

void SampleWidget::slotProgressionChanged()
{
	slotView(true);
}

void SampleWidget::slotWantShape()
{
	slotView ( true );
}


void SampleWidget::refillSampleList()
{
	ui->sampleTextTree->clear();

	QTreeWidgetItem * curIt = 0;
	QMap<QString, QList<QString> > sl = typotek::getInstance()->namedSamplesNames();
	QList<QString> ul( sl.take(QString("User")) );
	if(ul.count())
	{
		QTreeWidgetItem * uRoot = new QTreeWidgetItem(ui->sampleTextTree);
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
	if(ui->lightHinting->isChecked())
		return FT_LOAD_TARGET_LIGHT;
	else if(ui->normalHinting->isChecked())
		return FT_LOAD_TARGET_NORMAL;

	return FT_LOAD_NO_HINTING ;
}


void SampleWidget::slotPrint()
{
	FontItem * font(FMFontDb::DB()->Font( fontIdentifier ));
	if(!font)
		return;
	layoutForPrint = true;
	slotView(false);
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
