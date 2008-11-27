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
#include "fmfontdb.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

FmRepair::FmRepair(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	fillLists();
	doConnect();
	
}

FmRepair::~ FmRepair()
{
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
}

void FmRepair::fillLists()
{
	fillDeadLink();
	fillActNotLinked();
	fillDeactLinked();
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
		if(/*!flist[i]->isLocked() &&*/ !flist[i]->isRemote() && !flist[i]->isActivated())
			deactivated << flist[i]->path();
	}
	qDebug() << deactivated.join("\nDEACT : ");
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
			qDebug() << "NO " << linked[i] ;
			QListWidgetItem *lit = new QListWidgetItem(linked[i]);
			lit->setCheckState(Qt::Unchecked);
			lit->setToolTip(linked[i]);
			deactLinkList->addItem(lit);
		}
		else
		{
			qDebug() << "OK " << linked[i] ;
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



