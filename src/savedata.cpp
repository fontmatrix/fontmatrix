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
		writeStartElement("fontfile");
		writeStartElement("file");
		writeCharacters(fitem->name());
		writeEndElement();
		QStringList tl = fitem->tags();
		foreach(QString tag, tl)
		{
			writeStartElement("tag");
			writeCharacters( tag );
			writeEndElement();
		}
		writeEndElement();
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
	QStringList sampleT= m_typo->sampleText().split("\n");
	foreach(QString sline, sampleT)
	{
		writeStartElement("sampleline");
		writeCharacters( sline );
		writeEndElement();
	}
	
	// save preview word
	writeStartElement("previewword");
	writeCharacters( m_typo->word() );
	writeEndElement();
	
	
	writeEndElement();//fontmatrix
	writeEndDocument();
}


