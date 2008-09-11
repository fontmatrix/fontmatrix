//
// C++ Implementation: importtags
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "importtags.h"

#include <QDebug>

ImportTags::ImportTags(QWidget * parent, QStringList tags)
 : QDialog(parent)
{
	qDebug()<< "ImportTags(" << tags << ")";
	setupUi(this);
	m_tags = tags;
// 	m_tags.removeAll("Activated_On");
// 	m_tags.removeAll("Activated_Off");
	for(int i = 0 ; i < m_tags.count() ; ++i)
	{
		QListWidgetItem *it = new QListWidgetItem( m_tags[i] , tagsList );
		it->setCheckState(Qt::Unchecked);
	}
	
	connect(tagNewButton,SIGNAL(released()),this,SLOT(slotNewTag()));
	connect(tagText,SIGNAL(editingFinished()),this,SLOT(slotNewTag()));
	connect(okButton,SIGNAL(released()),this,SLOT(slotEnd()));
}


ImportTags::~ImportTags()
{
	for(int i = 0 ; i < tagsList->count() ; ++i)
	{
		delete tagsList->item(i);
	}
}

void ImportTags::slotNewTag()
{
	if(m_tags.contains(tagText->text()))
		return;
	if(tagText->text().simplified().isEmpty())
		return;
	m_tags << tagText->text();
	QListWidgetItem *it = new QListWidgetItem(tagText->text(),tagsList);
	it->setCheckState(Qt::Checked);	
	tagText->clear();
}

void ImportTags::slotEnd()
{
	m_tags.clear();
	for(int i = 0 ; i < tagsList->count() ; ++i)
	{
		if(tagsList->item(i)->checkState() == Qt::Checked)
			m_tags << tagsList->item(i)->text();
	}
	close();
}


