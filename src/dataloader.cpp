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
#include "dataloader.h"
#include "typotek.h"


#include <QDomDocument>
#include <QDomNodeList>
#include <QMessageBox>
#include <QString>
#include <QStatusBar>
#include <QDebug>

extern bool __FM_SHOW_FONTLOADED;

DataLoader::DataLoader(QFile *file )
	: m_file(file)
{
	m_typo = typotek::getInstance();
	
}


DataLoader::~DataLoader()
{
}

void DataLoader::load()
{
		
	QDomDocument doc ( "fontdata" );

	if ( !m_file->open ( QFile::ReadOnly ) )
	{
		// Ensure that there are default samples and preview text
		QStringList fallbackSample(QObject::tr("ABCDEFGH\nIJKLMNOPQ\nRSTUVXYZ\n\nabcdefgh\nijklmnopq\nrstuvxyz\n0123456789\n,;:!?.").split("\n"));
		for ( uint i = 0; i < fallbackSample.count(); ++i )
		{
			m_typo->addNamedSampleFragment(m_typo->defaultSampleName() , fallbackSample[i]);
		}	
		m_typo->setWord(QObject::tr("hamburgefonstiv"), false);
		return;
	}
	if ( !doc.setContent ( m_file ) )
	{
		m_file->close();
		m_typo->statusBar()->showMessage ( QString ( "ERROR loading xml for %1" ).arg ( m_file->fileName() ),3000 );
		return;
	}
	m_file->close();
	m_typo->statusBar()->showMessage ( QString ( "loading %1" ).arg ( m_file->fileName()) );
	
	QStringList collectedTags; 
	
	
	//loading fonts
	QDomNodeList colList = doc.elementsByTagName ( "fontfile" );
	qDebug()<< "colList contains "<< colList.count() << "fontfile elements";
	if ( colList.length() == 0 )
	{
		m_typo->statusBar()->showMessage ( QString ( "WARNING: no fontfile in %1" ).arg ( m_file->fileName() ),3000 );
	}

	for ( uint i = 0; i < colList.length(); ++i )
	{
		QDomNode col = colList.item ( i );
		FontInfo fi;
// 			QString fontName = col.toElement().attributeNode("name").value();
			fi.file  = col.namedItem ( "file" ).toElement().text();
			fi.family = col.toElement().attributeNode("family").value();
			fi.variant = col.toElement().attributeNode("variant").value();
			fi.type = col.toElement().attributeNode("type").value();
			fi.info = col.namedItem ( "info" ).toElement().text();
		QDomNodeList taglist = col.toElement().elementsByTagName ( "tag" );
		QStringList tl;
		for(int ti = 0; ti < taglist.count(); ++ti)
		{
			fi.tags << taglist.at(ti).toElement().text();
		}
		m_typo->addTagMapEntry(fi.file,fi.tags);
		collectedTags << fi.tags;
		m_fastList[fi.file] = fi;
// 		qDebug() << fi.file << fi.family << fi.variant;

	}
	
	
	//loading tagsets
	QDomNodeList setList = doc.elementsByTagName ( "tagset" );
	if ( setList.length() == 0 )
	{
		m_typo->statusBar()->showMessage ( QString ( "WARNING: no tagset in %1" ).arg ( m_file->fileName() ),3000 );
	}
	for ( uint i = 0; i < setList.length(); ++i )
	{
		QDomNode col = setList.item ( i );
		QString set = col.toElement().attributeNode("name").value();
		QDomNodeList taglist = col.toElement().elementsByTagName ( "tag" );
		QStringList tl;
		for(int ti = 0; ti < taglist.count(); ++ti)
		{
			tl << taglist.at(ti).toElement().text();
		}
		m_typo->addTagSetMapEntry(set,tl);
		collectedTags << tl;
		
// 		qDebug() << set << tl.join(":");
	}
	collectedTags.removeAll("");
	typotek::tagsList = collectedTags.toSet().toList();
	
	//loading sample text
	QString sampleText;
	QDomNodeList sampleList = doc.elementsByTagName ( "sampleline" );
	if ( sampleList.length() == 0 )
	{
		m_typo->statusBar()->showMessage ( QString ( "WARNING: no sample text in %1" ).arg ( m_file->fileName() ),3000 );
		QStringList fallbackSample(QObject::tr("ABCDEFGH\nIJKLMNOPQ\nRSTUVXYZ\n\nabcdefgh\nijklmnopq\nrstuvxyz\n0123456789\n,;:!?.").split("\n"));
		for ( uint i = 0; i < fallbackSample.count(); ++i )
		{
			m_typo->addNamedSampleFragment(m_typo->defaultSampleName(), fallbackSample[i]);
		}
	}
	else
	{
		for ( uint i = 0; i < sampleList.length(); ++i )
		{
			QDomNode col = sampleList.item ( i );
			QString name = col.toElement().attributeNode("name").value();
			if(name.isEmpty()
				|| name == "default")// rather to not break previous installation
				name = m_typo->defaultSampleName();
			m_typo->addNamedSampleFragment(name, col.toElement().text());
		}
	}

	//loading preview word
	QString pWord;
	QDomNodeList previewList = doc.elementsByTagName ( "previewword" );
	if ( previewList.length() == 0 )
	{
		m_typo->statusBar()->showMessage ( QString ( "WARNING: no preview word in %1" ).arg ( m_file->fileName() ),3000 );
		pWord =  QObject::tr("hamburgefonstiv");
	}
	else
	{
		QDomNode col = previewList.item ( 0 );
		pWord = col.toElement().text();
	}
	m_typo->setWord(pWord, false);
}


