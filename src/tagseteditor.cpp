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
#include "tagseteditor.h"
#include "typotek.h"

#include <QDebug>

TagSetEditor::TagSetEditor()
{
	setupUi(this);
	m_typo = typotek::getInstance();
	doConnect();
	doInitLists();
}


TagSetEditor::~TagSetEditor()
{
}

void TagSetEditor::doConnect()
{
	connect(closeButton,SIGNAL(released()),this,SLOT(close()));
	connect(closeButton,SIGNAL(released()),m_typo,SLOT(save()));
	connect(newSetButton,SIGNAL(released()),this,SLOT(slotNewSet()));
	connect(addToSetButton,SIGNAL(released()),this,SLOT(slotAddTagToSet()));
	connect(removeToSetButton,SIGNAL(released()),this,SLOT(slotRemoveToSet()));
	connect(setList,SIGNAL(itemPressed( QListWidgetItem* )),this,SLOT(slotUpdateTagsOfSet( QListWidgetItem*)));
}

void TagSetEditor::doInitLists()
{
	QStringList tl = typotek::tagsList;
	tl.removeAll("Activated_Off");
	tl.removeAll("Activated_On");
	tagList->addItems(tl);
	QStringList sets = m_typo->tagsets();
	setList->addItems(sets);
	if(sets.count() && !sets.at(0).isEmpty())
	{	
		
		QListWidgetItem *defaultItem = setList->item(0);
		defaultItem->setSelected(true);
		QStringList ta = m_typo->tagsOfSet(sets.at(0));
		tagsOfSetList->addItems(ta);
	}
}

void TagSetEditor::slotNewSet()
{
	if(newSetText->text().isEmpty())
		return;
	if(m_typo->tagsets().contains(newSetText->text()))
		  return;
	
	setList->addItem(newSetText->text());
	tagsOfSetList->clear();
	m_typo->addTagSetMapEntry(newSetText->text(), QStringList());
	setList->item(setList->count() - 1)->setSelected(true) ;
	emit signalNewTagset();
	newSetText->clear();
	
}

void TagSetEditor::slotAddTagToSet()
{
	QString curSet;
	for(int i = 0; i < setList->count();++i)
	{
		if(setList->item(i)->isSelected())
			curSet = setList->item(i)->text();
	}
	
	if(curSet.isEmpty())
		return;
	
	QStringList sel;
	for(int i = 0; i < tagList->count();++i)
	{
		QListWidgetItem *it = tagList->item(i);
		if(it->isSelected())
			sel << it->text();
	}
	
	foreach(QString it, sel)
	{
		if(!m_typo->tagsOfSet(curSet).contains(it))
		{
			tagsOfSetList->addItem(it);
		}
	}
	m_typo->addTagSetMapEntry(curSet, sel);
}

void TagSetEditor::slotRemoveToSet()
{
	QString curSet;
	for(int i = 0; i < setList->count();++i)
	{
		if(setList->item(i)->isSelected())
			curSet = setList->item(i)->text();
	}
	
	if(curSet.isEmpty())
		return;
	
	QList<QListWidgetItem*> sel;
	QStringList rest;
	for(int i = 0; i < tagsOfSetList->count();++i)
	{
		QListWidgetItem *it = tagsOfSetList->item(i);
		if(it->isSelected())
			sel << it;
		else
			rest << it->text();
	}
	foreach(QListWidgetItem * it, sel)
	{	
			m_typo->removeTagFromSet(curSet, it->text());
	}
	tagsOfSetList->clear();
	tagsOfSetList->addItems(rest);
}


void TagSetEditor::slotUpdateTagsOfSet( QListWidgetItem* item )
{
	tagsOfSetList->clear();
	tagsOfSetList->addItems(m_typo->tagsOfSet(item->text()));
}


