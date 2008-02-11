//
// C++ Implementation: dataexport
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dataexport.h"

#include "typotek.h"
#include "fontitem.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QProgressDialog>

DataExport::DataExport(const QString &dirPath, const QString &filterTag)
{
	exDir.setPath(dirPath);
	filter = filterTag;
	typotek *typ = typotek::getInstance();
	fonts = typ->getFonts(filter,"tag");
}


DataExport::~DataExport()
{
}

int DataExport::copyFiles()
{
	QProgressDialog progress ( QObject::tr ( "Copying files" ), QObject::tr ( "cancel" ), 0, fonts.count(), typotek::getInstance() );
	progress.setWindowModality ( Qt::WindowModal );
	int progressindex(0);
	
	int copyCounter(0);
	QList<int> toRemove;
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		if ( progress.wasCanceled() )
			break;
		progress.setLabelText ( fonts[fidx]->fancyName() );
		progress.setValue ( ++progressindex );
		
		QFile ffile(fonts[fidx]->path());
		QFileInfo ifile(ffile);
// 		qDebug()<< exDir.absolutePath() + exDir.separator() + ifile.fileName();
		if(ffile.copy(exDir.absolutePath() + exDir.separator() + ifile.fileName()) )
		{
			++copyCounter;
		}
		else
		{
			typotek::getInstance()->showStatusMessage(QObject::tr("Unable to copy")+" "+fonts[fidx]->path());
			toRemove << fidx;
		}
	}
	
	for(int i(toRemove.count() - 1); i >= 0;--i)
		fonts.removeAt(toRemove[i]);
	return copyCounter;
}

int DataExport::buildIndex()
{
	QFile file(exDir.absolutePath() + exDir.separator() +"fontmatrix.data");
	QXmlStreamWriter xmlStream(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Export Warning : Can't open " << file.fileName();
		return 0;
	}
	else
	{
		xmlStream.setAutoFormatting(true);
	}
	
	xmlStream.writeStartDocument();
	xmlStream.writeStartElement("fontmatrix");
	xmlStream.writeAttribute("version", "1.0");
	
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		FontItem* fitem(fonts[fidx]);
		{
			xmlStream.writeStartElement("fontfile");
			xmlStream.writeAttribute("family", fitem->family());
			xmlStream.writeAttribute("variant",fitem->variant());
			xmlStream.writeStartElement("file");
			xmlStream.writeCharacters( QFileInfo(fitem->path()).fileName() );
			xmlStream.writeEndElement();
			QStringList tl = fitem->tags();
			foreach(QString tag, tl)
			{
				xmlStream.writeStartElement("tag");
				xmlStream.writeCharacters( tag );
				xmlStream.writeEndElement();
			}
			xmlStream.writeEndElement();
		}		
	}
	
	xmlStream.writeEndElement();//fontmatrix
	xmlStream.writeEndDocument();
	file.close();
	return fonts.count();
}

int DataExport::doExport()
{
	int cp (copyFiles());
	int bd (buildIndex());
	
	if( bd == 0 )
		return 0;
	
	return cp;
}


