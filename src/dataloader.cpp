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
		QMessageBox::warning (0, QString ( "Application" ),
				       QString ( "Cannot read file %1:\n%2." )
						       .arg ( m_file->fileName() )
						       .arg ( m_file->errorString() ) );
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
	qDebug() << colList.length();
	for ( uint i = 0; i < colList.length(); ++i )
	{
		QDomNode col = colList.item ( i );
		QString fontfile  = col.namedItem ( "file" ).toElement().text();
		QDomNodeList taglist = col.toElement().elementsByTagName ( "tag" );
		QStringList tl;
		for(int ti = 0; ti < taglist.count(); ++ti)
		{
			tl << taglist.at(ti).toElement().text();
		}
		m_typo->addTagMapEntry(fontfile,tl);
		collectedTags << tl;
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
		
		qDebug() << set << tl.join(":");
	}
	
	typotek::tagsList = collectedTags.toSet().toList();
}


