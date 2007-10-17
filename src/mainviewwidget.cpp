/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
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
#include "mainviewwidget.h"
#include "typotek.h"
#include "fontitem.h"
#include "fontactionwidget.h"
#include "typotekadaptator.h"

#include <QString>
#include <QDebug>
#include <QGraphicsItem>
#include <QTransform>
#include <QInputDialog>



MainViewWidget::MainViewWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	
	typo = reinterpret_cast<typotek*>(parent);
	
	currentFaction =0;
	abcScene = new QGraphicsScene;
	loremScene = new QGraphicsScene;
	
	abcView->setScene(abcScene);
	loremView->setScene(loremScene);
// 	loremView->scale(0.5,0.5);
	
	sampleText = "Here, type your own\nlorem ipsum";
	
	ord << "family" << "variant";
	orderingCombo->addItems(ord);
	
	fields << "family" << "variant";
	searchField->addItems(fields);
	
	QStringList tl_tmp = typotek::tagsList;
	qDebug() << "TAGLIST\n" << typotek::tagsList.join("\n");
	tl_tmp.removeAll("Activated_On");
	tl_tmp.removeAll("Activated_Off");
	
	tagsCombo->addItems(tl_tmp);
	
	connect(orderingCombo,SIGNAL(activated( const QString )),this,SLOT(slotOrderingChanged(QString)));
	
	connect(fontTree,SIGNAL(itemClicked( QTreeWidgetItem*, int )),this,SLOT(slotfontSelected(QTreeWidgetItem*, int)));
	connect(fontTree,SIGNAL(itemClicked( QTreeWidgetItem*, int )),this,SLOT(slotFontAction(QTreeWidgetItem*, int)));
	connect(editAllButton,SIGNAL(clicked( bool )),this,SLOT(slotEditAll()));
	
	connect(this,SIGNAL(faceChanged()),this,SLOT(slotInfoFont()));
	connect(this,SIGNAL(faceChanged()),this,SLOT(slotView()));
	
	connect(abcScene,SIGNAL(selectionChanged()),this,SLOT(slotglyphInfo()));
	connect(searchButton,SIGNAL(clicked( bool )),this,SLOT(slotSearch()));
	
	
	connect(renderZoom,SIGNAL(valueChanged( int )),this,SLOT(slotZoom(int)));
	connect(allZoom,SIGNAL(valueChanged( int )),this,SLOT(slotZoom(int)));
	
	connect(tagsCombo,SIGNAL(activated( const QString& )),this,SLOT(slotFilterTag(QString)));
	
	connect(activateAllButton,SIGNAL(released()),this,SLOT(slotActivateAll()));
	connect(desactivateAllButton,SIGNAL(released()),this,SLOT(slotDesactivateAll()));
	
	connect(textButton,SIGNAL(released()),this,SLOT(slotSetSampleText()));
	
	
}


MainViewWidget::~MainViewWidget()
{
}

void MainViewWidget::slotOrderingChanged(QString s)
{
	//Update "fontTree"
	fontTree->clear();

	currentFonts = typo->getAllFonts();
	QMap<QString, QList<FontItem*> > keyList;
	for(int i=0; i < currentFonts.count();++i)
	{
		keyList[currentFonts[i]->value(s)].append(currentFonts[i]);
	}
	QMap<QString, QList<FontItem*> >::const_iterator kit;
	for(kit = keyList.begin(); kit != keyList.end(); ++kit)
	{
		QTreeWidgetItem *ord = new QTreeWidgetItem(fontTree);
		ord->setText(0, kit.key());
		for(int  n = 0; n < kit.value().count(); ++n)
		{
			QTreeWidgetItem *entry = new QTreeWidgetItem(ord);
			entry->setText(1, kit.value()[n]->name());
		}
		fontTree->addTopLevelItem(ord);
	}
	
}

void MainViewWidget::slotfontSelected(QTreeWidgetItem * item, int column)
{
	qDebug() << "font select";
	
		lastIndex = faceIndex;
		faceIndex = item->text(1);
		qDebug() << faceIndex;
		if(faceIndex.count())
			emit faceChanged();
	
	
}

void MainViewWidget::slotInfoFont()
{
	FontItem *f = typo->getFont(faceIndex);
	fontInfoText->clear();
	//QString t(QString("Family : %1\nStyle : %2\nFlags : \n%3").arg(f->family()).arg(f->variant()).arg(f->faceFlags()));
	fontInfoText->setText(f->infoText());
	
}

void MainViewWidget::slotView()
{
	FontItem *l = typo->getFont(lastIndex);
	FontItem *f = typo->getFont(faceIndex);
	if(l)
		l->deRenderAll();
	f->deRenderAll();
	f->renderAll(abcScene);
	
	QStringList stl = sampleText.split('\n');
	for(int i=0; i< stl.count(); ++i)
		f->renderLine(loremScene,stl[i],25*i);}

void MainViewWidget::slotglyphInfo()
{
	if(abcScene->selectedItems().isEmpty())
		return;
	glyphInfo->clear();
	QString is = typo->getFont(faceIndex)->infoGlyph(abcScene->selectedItems()[0]->data(1).toInt(), abcScene->selectedItems()[0]->data(2).toInt());
	glyphInfo->setText(is);
}

void MainViewWidget::slotSearch()
{
	fontTree->clear();
	
	QString fs(searchString->text());
	QString ff(searchField->currentText());
	
	currentFonts = typo->getFonts(fs,ff) ;

	QMap<QString,QList<FontItem*> > keyList;
	for(int i=0;i <currentFonts.count();++i)
	{
		keyList[currentFonts[i]->value(orderingCombo->currentText())].append(currentFonts[i]);
	}
	
	QMap<QString, QList<FontItem*> >::const_iterator kit;
	for(kit = keyList.begin(); kit != keyList.end(); ++kit)
	{
		QTreeWidgetItem *ord = new QTreeWidgetItem(fontTree);
		ord->setText(0, kit.key());
		QList<FontItem*> alist = kit.value();
		for(int  n = 0; n < alist.count(); ++n)
		{
			QTreeWidgetItem *entry = new QTreeWidgetItem(ord);
			entry->setText(1, alist[n]->name());
		}
		fontTree->addTopLevelItem(ord);
	}
}

void MainViewWidget::slotFilterTag(QString tag)
{
	fontTree->clear();
	
	QString fs(tag);
	QString ff("tag");
	
	currentFonts = typo->getFonts(fs,ff) ;

	QMap<QString,QList<FontItem*> > keyList;
	for(int i=0;i <currentFonts.count();++i)
	{
		keyList[currentFonts[i]->value(orderingCombo->currentText())].append(currentFonts[i]);
	}
	
	QMap<QString, QList<FontItem*> >::const_iterator kit;
	for(kit = keyList.begin(); kit != keyList.end(); ++kit)
	{
		QTreeWidgetItem *ord = new QTreeWidgetItem(fontTree);
		ord->setText(0, kit.key());
		QList<FontItem*> alist = kit.value();
		for(int  n = 0; n < alist.count(); ++n)
		{
			QTreeWidgetItem *entry = new QTreeWidgetItem(ord);
			entry->setText(1, alist[n]->name());
		}
		fontTree->addTopLevelItem(ord);
	}
}


void MainViewWidget::slotFontAction(QTreeWidgetItem * item, int column)
{
	if (!currentFaction)
	{
		currentFaction = new FontActionWidget(typo->adaptator(), tagPage);
		connect(currentFaction,SIGNAL(cleanMe()),this,SLOT(slotCleanFontAction()));
		connect(currentFaction,SIGNAL(tagAdded(QString)),this,SLOT(slotAppendTag(QString)));
		currentFaction->show();
	}
	
	FontItem * FoIt = typo->getFont( item->text(1) );
	if(FoIt/* && (!FoIt->isLocked())*/)
	{
// 		currentFaction->slotFinalize();
		QList<FontItem*> fl;
		fl.append(FoIt);
		currentFaction->prepare(fl);
		
		
	}
}

void MainViewWidget::slotEditAll()
{
	if (!currentFaction)
	{
		currentFaction = new FontActionWidget(typo->adaptator(), tagPage);
		connect(currentFaction,SIGNAL(cleanMe()),this,SLOT(slotCleanFontAction()));
		connect(currentFaction,SIGNAL(tagAdded(QString)),this,SLOT(slotAppendTag(QString)));
		currentFaction->show();
	}
	
	
	QList<FontItem*> fl;
	for(int i =0; i< currentFonts.count(); ++i)
	{
// 		if(!currentFonts[i]->isLocked())
// 		{
			fl.append(currentFonts[i]);
// 		}
	}
	if(fl.isEmpty())
		return;
	
	currentFaction->prepare(fl);
}

void MainViewWidget::slotCleanFontAction()
{
	typo->save();
	qDebug() << " FontActionWidget  saved";
}

void MainViewWidget::slotZoom(int z)
{
	QGraphicsView * concernedView;
	if(sender()->objectName().contains("render"))
		concernedView = loremView;
	else
		concernedView = abcView;
	
	QTransform trans;
	double delta = (double)z / 100.0;
	trans.scale(delta,delta);
	concernedView->setTransform(trans, false);

}

void MainViewWidget::slotAppendTag(QString tag)
{
	qDebug() << "add tag to combo " << tag;
	tagsCombo->addItem(tag);
}

void MainViewWidget::allActivation(bool act)
{
	if(act)
	{
		foreach(FontItem* fit, currentFonts)
		{
			if(!fit->isLocked())
			{
				QStringList tl = fit->tags();
				if(!tl.contains("Activated_On"))
				{
					tl.removeAll("Activated_Off");
					tl << "Activated_On";
					fit->setTags(tl);
					
					QFileInfo fofi(fit->path());
					if(!QFile::link( fit->path() , QDir::home().absolutePath() + "/.fonts/" + fofi.baseName()))
					{
						qDebug() << "unable to link " << fofi.fileName();
					}
					else
					{
						typo->adaptator()->private_signal(1, fofi.fileName());
					}
				}
				
			}
		}
	}
	else
	{
		foreach(FontItem* fit, currentFonts)
		{
			if(!fit->isLocked())
			{
				QStringList tl = fit->tags();
				if(!tl.contains("Activated_Off"))
				{
					tl.removeAll("Activated_On");
					tl << "Activated_Off";
					fit->setTags(tl);
					
					QFileInfo fofi(fit->path());
					if(!QFile::remove( QDir::home().absolutePath() + "/.fonts/" + fofi.baseName()))
					{
						qDebug() << "unable to remove " << fofi.fileName();
					}
					else
					{
						typo->adaptator()->private_signal(0, fofi.fileName());
					}
				}
				
			}
		}
	}
}

void MainViewWidget::slotDesactivateAll()
{
	allActivation(false);
}

void MainViewWidget::slotActivateAll()
{
	allActivation(true);
}

void MainViewWidget::slotSetSampleText()
{
	sampleText = QInputDialog::getText(this, "typotek - getText", "Type your sample text here",QLineEdit::Normal,sampleText);
}


