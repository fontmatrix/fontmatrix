//
// C++ Implementation: fmrepair
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmrepair.h"
#include "typotek.h"
#include "fontitem.h"
#include "fmfontdb.h"
#include "mainviewwidget.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

FmRepair::FmRepair(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
// 	listItems.clear();
	fillLists();
	doConnect();
	
}

FmRepair::~ FmRepair()
{
// 	foreach(QListWidgetItem* lit , listItems)
// 	{
// 		if(lit)
// 			delete lit;
// 	}
}

void FmRepair::doConnect()
{
	connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
	
	connect(selectAllDead,SIGNAL(clicked()),this,SLOT(slotSelAllDead()));
	connect(removeDead,SIGNAL(clicked()),this,SLOT(slotRemoveDead()));
	
	connect(selectAllActNot,SIGNAL(clicked()),this,SLOT(slotSelAllActNotLinked()));
	connect(relinkActNot,SIGNAL(clicked()),this,SLOT(slotRelinkActNotLinked()));
	connect(deactActNot,SIGNAL(clicked()),this,SLOT(slotDeactivateActNotLinked()));
	
	connect(selectAllDeactLink,SIGNAL(clicked()),this,SLOT(slotSelAllDeactLinked()));
	connect(delinkDeactLink,SIGNAL(clicked()),this,SLOT(slotDelinkDeactLinked()));
	connect(activateDeactLink,SIGNAL(clicked()),this,SLOT(slotActivateDeactLinked()));
	
	connect(selectAllUnreferenced,SIGNAL(clicked()),this,SLOT(slotSelectAllUnref()));
	connect(removeUnreferenced,SIGNAL(clicked()),this,SLOT(slotRemoveUnref()));
}

void FmRepair::fillLists()
{
	fillDeadLink();
	fillActNotLinked();
	fillDeactLinked();
	fillUnreferenced();
}

void FmRepair::fillDeadLink()
{
	typotek *t = typotek::getInstance();
	deadList->clear();
	QDir md(t->getManagedDir());
	md.setFilter( QDir::Files );
	QFileInfoList list = md.entryInfoList();
	for(int i(0); i < list.count(); ++i)
	{
		if(list[i].isSymLink())
		{
			if( !QFileInfo(list[i].symLinkTarget()).exists() )
			{
				QListWidgetItem *lit = new QListWidgetItem(list[i].absoluteFilePath());
				lit->setCheckState(Qt::Unchecked);
				lit->setToolTip(list[i].absoluteFilePath());
				deadList->addItem(lit);
// 				listItems << lit;
			}
		}
	}
	
}

void FmRepair::fillActNotLinked()
{
	typotek *t = typotek::getInstance();
	actNotLinkList->clear();
	QList<FontItem*> flist(FMFontDb::DB()->AllFonts());
	QStringList activated;
	for(int i(0); i < flist.count();++i)
	{
		if((!t->isSysFont(flist[i])) && (flist[i]->isActivated()))
			activated << flist[i]->path();
	}
	
	QStringList linked;
	QDir md(t->getManagedDir());
	md.setFilter( QDir::Files );
	QFileInfoList list = md.entryInfoList();
	for(int i(0); i < list.count(); ++i)
	{
		if(list[i].isSymLink())
		{
			if(  QFileInfo(list[i].symLinkTarget()).exists()  )
			{
// 				qDebug()<< "ACT NOT LINK "<<list[i].symLinkTarget();
				linked << list[i].symLinkTarget();
			}
			else
			{
				qDebug()<<list[i].filePath()<<" is a broken symlink";
			}
		}
		else
		{
			qDebug()<<list[i].filePath() << " is not a symlink";
		}
	}
	
	for(int i(0); i < activated.count(); ++i)
	{
		if(!linked.contains(activated[i]))
		{
			QListWidgetItem *lit = new QListWidgetItem(activated[i]);
			lit->setCheckState(Qt::Unchecked);
			lit->setToolTip(activated[i]);
			actNotLinkList->addItem(lit);
// 			listItems << lit;
		}
	}
}

void FmRepair::fillDeactLinked()
{
	typotek *t = typotek::getInstance();
	deactLinkList->clear();
	QList<FontItem*> flist(FMFontDb::DB()->AllFonts());
	QStringList deactivated;
	for(int i(0); i < flist.count();++i)
	{
		if(!t->isSysFont(flist[i]) && !flist[i]->isRemote() && !flist[i]->isActivated())
			deactivated << flist[i]->path();
	}
// 	qDebug() << deactivated.join("\nDEACT : ");
	QStringList linked;
	QDir md(t->getManagedDir());
	md.setFilter( QDir::Files );
	QFileInfoList list = md.entryInfoList();
	for(int i(0); i < list.count(); ++i)
	{
		if(list[i].isSymLink())
		{
			if(  QFileInfo(list[i].symLinkTarget()).exists()  )
			{
				linked << list[i].symLinkTarget();
			}
		}
	}
	
	for(int i(0); i < linked.count(); ++i)
	{
		if(deactivated.contains(linked[i]))
		{
// 			qDebug() << "NO " << linked[i] ;
			QListWidgetItem *lit = new QListWidgetItem(linked[i]);
			lit->setCheckState(Qt::Unchecked);
			lit->setToolTip(linked[i]);
			deactLinkList->addItem(lit);
// 			listItems << lit;
		}
		else
		{
// 			qDebug() << "OK " << linked[i] ;
		}
	}
}

void FmRepair::slotSelAllDead()
{
	for(int i(0); i < deadList->count(); ++i)
	{
		deadList->item(i)->setCheckState(Qt::Checked);
	}
}

void FmRepair::slotRemoveDead()
{
	for(int i(0); i < deadList->count(); ++i)
	{
		if(deadList->item(i)->checkState() == Qt::Checked)
		{
			QFile f(deadList->item(i)->text());
			f.remove();
		}
	}
	fillDeadLink();
}

void FmRepair::slotSelAllActNotLinked()
{
	for(int i(0); i < actNotLinkList->count(); ++i)
	{
		actNotLinkList->item(i)->setCheckState(Qt::Checked);
	}
}

void FmRepair::slotRelinkActNotLinked()
{
	typotek *t = typotek::getInstance();
	for(int i(0); i < actNotLinkList->count() ; ++i)
	{
		if(actNotLinkList->item(i)->checkState() == Qt::Checked)
		{
			FontItem *font = 0;
			if(font = FMFontDb::DB()->Font(actNotLinkList->item(i)->text()))
			{
				QFile f(font->path());
				f.link( t->getManagedDir() + QDir::separator() + font->activationName() );
				
					if(!font->afm().isEmpty())
					{
						QFile af(font->afm());
						af.link( t->getManagedDir() + QDir::separator() + font->activationAFMName() );
					}
			}
		}
	}
	fillActNotLinked();
}

void FmRepair::slotDeactivateActNotLinked()
{
	typotek *t = typotek::getInstance();
	for(int i(0); i < actNotLinkList->count() ; ++i)
	{
		if(actNotLinkList->item(i)->checkState() == Qt::Checked)
		{
			FontItem *font = 0;
			if(font = FMFontDb::DB()->Font(actNotLinkList->item(i)->text()))
			{
				font->setActivated(false);
			}
		}
	}
	
	fillActNotLinked();
}

void FmRepair::slotSelAllDeactLinked()
{
	for(int i(0); i < deactLinkList->count(); ++i)
	{
		deactLinkList->item(i)->setCheckState(Qt::Checked);
	}
}

void FmRepair::slotDelinkDeactLinked()
{
	typotek *t = typotek::getInstance();
	for(int i(0); i < deactLinkList->count(); ++i)
	{
		if(deactLinkList->item(i)->checkState() == Qt::Checked)
		{
			FontItem *font = 0;
			if(font = FMFontDb::DB()->Font(deactLinkList->item(i)->text()))
			{
				QFile f(t->getManagedDir() + QDir::separator() + font->activationName());
				f.remove();
			}
		}
	}
	fillDeactLinked();
}

void FmRepair::slotActivateDeactLinked()
{
	typotek *t = typotek::getInstance();
	
	for(int i(0); i < deactLinkList->count(); ++i)
	{
		if(deactLinkList->item(i)->checkState() == Qt::Checked)
		{
			FontItem *font = 0;
			if(font = FMFontDb::DB()->Font(deactLinkList->item(i)->text()))
			{
				font->setActivated(true);
			}
		}
	}
	fillDeactLinked();
}





void FmRepair::fillUnreferenced()
{
	unrefList->clear();
	foreach(const QString& fid, FMFontDb::DB()->AllFontNames())
	{
		if(!QFile::exists(fid))
		{
			QListWidgetItem *lit = new QListWidgetItem(fid);
			lit->setCheckState(Qt::Unchecked);
			unrefList->addItem(lit);
// 			listItems << lit;
		}
	}
}

void FmRepair::slotSelectAllUnref()
{
	for(int i(0); i < unrefList->count(); ++i)
	{
		unrefList->item(i)->setCheckState(Qt::Checked);
	}
}

void FmRepair::slotRemoveUnref()
{
	FMFontDb *db(FMFontDb::DB());
	QStringList failed;
	QList<FontItem*> flist(typotek::getInstance()->getTheMainView()->curFonts());
	for(int i(0); i < unrefList->count(); ++i)
	{
		if(unrefList->item(i)->checkState() == Qt::Checked)
		{
			FontItem* curItem = 0;
			QString fId(unrefList->item(i)->text());
			foreach(FontItem* it, flist)
			{
				if(it->path() == fId)
				{
					curItem = it;
					break;
				}
			}
			if(!db->Remove(fId))
				failed << fId;
			else if(curItem)
				flist.removeAll(curItem);
			
		}
	}
	typotek::getInstance()->getTheMainView()->setCurFonts(flist);
// 	if(failed.count() > 0)
// 	{
// 		QMessageBox::warning(this,"Fontmatrix - warning",failed.join("\n"));
// 	}
	fillUnreferenced();
}