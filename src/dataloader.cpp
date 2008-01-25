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
		m_typo->setSampleText( "ABCDEFGH\nIJKLMNOPQ\nRSTUVXYZ\n\nabcdefgh\nijklmnopq\nrstuvxyz\n0123456789\n,;:!?.");	
		m_typo->setWord("hamburgefonstiv", false);
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
	if ( colList.length() == 0 )
	{
		m_typo->statusBar()->showMessage ( QString ( "WARNING: no fontfile in %1" ).arg ( m_file->fileName() ),3000 );
	}
// 	qDebug() << colList.length();
	if(__FM_SHOW_FONTLOADED)
	{
		for ( uint i = 0; i < colList.length(); ++i )
		{
			QDomNode col = colList.item ( i );
#if FONTMATRIX_VERSION_MINOR == 3
			QString fontName  = col.namedItem ( "file" ).toElement().text();
			QString fontfile = QDir::home() + "/.fonts-reserved/"+ fontName;
#else
			QString fontName = col.toElement().attributeNode("name").value();
			QString fontfile  = col.namedItem ( "file" ).toElement().text();
#endif
			QDomNodeList taglist = col.toElement().elementsByTagName ( "tag" );
			QStringList tl;
			for(int ti = 0; ti < taglist.count(); ++ti)
			{
				tl << taglist.at(ti).toElement().text();
			}
			if(!m_fontList.contains(fontfile))
			{
				m_typo->addTagMapEntry(fontName,tl);
				collectedTags << tl;
				m_fontList << fontfile;
				m_typo->relayStartingStepIn(fontName, Qt::AlignCenter , Qt::white );
			}
		}
	}
	else
	{
		for ( uint i = 0; i < colList.length(); ++i )
		{
			QDomNode col = colList.item ( i );
#if FONTMATRIX_VERSION_MINOR == 3
			QString fontName  = col.namedItem ( "file" ).toElement().text();
			QString fontfile = QDir::home() + "/.fonts-reserved/"+ fontName;
#else
			QString fontName = col.toElement().attributeNode("name").value();
			QString fontfile  = col.namedItem ( "file" ).toElement().text();
#endif
			QDomNodeList taglist = col.toElement().elementsByTagName ( "tag" );
			QStringList tl;
			for(int ti = 0; ti < taglist.count(); ++ti)
			{
				tl << taglist.at(ti).toElement().text();
			}
			if(!m_fontList.contains(fontfile))
			{
				m_typo->addTagMapEntry(fontName,tl);
				collectedTags << tl;
				m_fontList << fontfile;
			}
		}
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
		m_typo->setSampleText("ABCDEFGH\nIJKLMNOPQ\nRSTUVXYZ\n\nabcdefgh\nijklmnopq\nrstuvxyz\n0123456789\n,;:!?.");
	}
	else
	{
		for ( uint i = 0; i < sampleList.length(); ++i )
		{
			QDomNode col = sampleList.item ( i );
			QString name = col.toElement().attributeNode("name").value();
			if(name.isEmpty())
				name = "default";
			m_typo->addNamedSampleFragment(name, col.toElement().text());
		}
	}

	//loading preview word
	QString pWord;
	QDomNodeList previewList = doc.elementsByTagName ( "previewword" );
	if ( previewList.length() == 0 )
	{
		m_typo->statusBar()->showMessage ( QString ( "WARNING: no preview word in %1" ).arg ( m_file->fileName() ),3000 );
		pWord =  "hamburgefonstiv";
	}
	else
	{
		QDomNode col = previewList.item ( 0 );
		pWord = col.toElement().text();
	}
	m_typo->setWord(pWord, false);
}


