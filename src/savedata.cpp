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
#include "savedata.h"
#include "typotek.h"
#include "fontitem.h"

#include <QDebug>

SaveData::SaveData(QFile *file,typotek *typo)
 : QXmlStreamWriter(file), m_typo(typo)
{
	if (!file->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Saver Warning : Can't open " << file->fileName();
	}
	else
	{
		setAutoFormatting(true);
		doSave();
		file->close();
	}
	
}


SaveData::~SaveData()
{
}

void SaveData::doSave()
{
	writeStartDocument();
	writeStartElement("fontmatrix");
	writeAttribute("version", "1.0");
	
	//save fonts
	QList<FontItem*> flist = m_typo->getAllFonts();
	foreach ( FontItem* fitem,flist )
	{
		if(!fitem->isLocked() && !fitem->isRemote())
		{
			writeStartElement("fontfile");
			writeAttribute("family", fitem->family());
			writeAttribute("variant",fitem->variant());
			writeAttribute("type",fitem->type());
			writeAttribute("name", fitem->fancyName());
			if(!fitem->panose().isEmpty())
				writeAttribute("panose", fitem->panose());
			writeStartElement("file");
			writeCharacters(fitem->path());
			writeEndElement();
			
			writeStartElement("info");
			
			QMap<int, QMap<QString,QString> > Info(fitem->rawInfo());
			QMap<int, QMap<QString,QString> >::const_iterator infoMap = Info.begin();
			QMap<int, QMap<QString,QString> >::const_iterator endMap = Info.end();
			
// 			qDebug() << "INFO" << Info.count() << infoMap.key()  ;
			
			while( infoMap != endMap )
			{
// 				qDebug() << "LANG" << infoMap.key() ;
				writeStartElement("lang");
				writeAttribute("code", QString::number( infoMap.key() ) );
				QMap<QString,QString>::const_iterator langMap = infoMap.value().begin();
				QMap<QString,QString>::const_iterator endLang = infoMap.value().end();
				
				for(;langMap != endLang; ++langMap)
				{
					writeStartElement("name");
					writeAttribute( "name" ,langMap.key()); // ;-)
					writeCharacters(langMap.value());
					writeEndElement(); // key
				}
				writeEndElement();//lang
				
				++infoMap;
			}
			
			writeEndElement();//info
			QStringList tl = fitem->tags();
			foreach(QString tag, tl)
			{
				writeStartElement("tag");
				writeCharacters( tag );
				writeEndElement();
			}
			writeEndElement();
		}
	}

	
	//save tagsets
	QStringList tlist = m_typo->tagsets();
	foreach(QString tagset, tlist)
	{
		
		writeStartElement("tagset");
		writeAttribute("name", tagset);
		QStringList tl = m_typo->tagsOfSet(tagset);
// 		qDebug()<<tagset <<" : "<< tl.join("+");
		foreach(QString tag, tl)
		{
			writeStartElement("tag");
			writeCharacters( tag );
			writeEndElement();
		}
		writeEndElement();
	}
	
	//save sample text
	foreach(QString samplename, m_typo->namedSamplesNames())
	{
		QStringList sampleT= m_typo->namedSample(samplename).split("\n");
		foreach(QString sline, sampleT)
		{
			writeStartElement("sampleline");
			writeAttribute("name", samplename);
			writeCharacters( sline );
			writeEndElement();
		}
	}
	
	// save preview word
	writeStartElement("previewword");
	writeCharacters( m_typo->word() );
	writeEndElement();
	

	writeEndElement();//fontmatrix
	writeEndDocument();
}


