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
#include "fmfontdb.h"
#include "fminfodisplay.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QProgressDialog>

DataExport::DataExport(const QString &dirPath, const QString &filterTag)
{
	exDir.setPath(dirPath);
	filter = filterTag;
	typotek *typ = typotek::getInstance();
// 	fonts = typ->getFonts(filter,"tag");
	fonts = FMFontDb::DB()->Fonts(filter,FMFontDb::Tags);
	typ->resetFilter();
}


DataExport::~DataExport()
{
}

int DataExport::copyFiles()
{
	QProgressDialog progress ( QObject::tr ( "Copying files" ), QObject::tr ( "cancel" ), 0, fonts.count(), typotek::getInstance() );
	progress.setWindowModality ( Qt::WindowModal );
	int progressindex(0);
	
	QString preview(typotek::getInstance()->word());
	
	int copyCounter(0);
	QList<int> toRemove;
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		if ( progress.wasCanceled() )
			break;
		
		preview.replace("<name>", fonts[fidx]->fancyName());
		preview.replace("<family>", fonts[fidx]->family());
		preview.replace("<variant>", fonts[fidx]->variant());
		
		progress.setLabelText ( fonts[fidx]->fancyName() );
		progress.setValue ( ++progressindex );
		
		QFile ffile(fonts[fidx]->path());
		QFileInfo ifile(ffile);
// 		qDebug()<< exDir.absolutePath() + exDir.separator() + ifile.fileName();
		if(ffile.copy(exDir.absolutePath() + exDir.separator() + ifile.fileName()) )
		{
			++copyCounter;
			QImage itImage(fonts[fidx]->oneLinePreviewPixmap(preview, Qt::white).toImage());
			if(!itImage.save(exDir.absolutePath() + exDir.separator() + ifile.fileName() + ".png"))
				qDebug()<<"Unable to save "<< exDir.absolutePath() + exDir.separator() + ifile.fileName() + ".png";
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
	xmlStream.writeAttribute("version", "1.1");
	
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		FontItem* fitem(fonts[fidx]);
		{
			xmlStream.writeStartElement("fontfile");
			xmlStream.writeAttribute("family", fitem->family());
			xmlStream.writeAttribute("variant",fitem->variant());
			xmlStream.writeAttribute("type",fitem->type());
			xmlStream.writeStartElement("file");
			xmlStream.writeCharacters( QFileInfo(fitem->path()).fileName() );
			xmlStream.writeEndElement();
			xmlStream.writeStartElement("info");
			FMInfoDisplay fid(fitem);
			xmlStream.writeCharacters( fid.getHtml() );
			xmlStream.writeEndElement();
			QStringList tl = fitem->tags();
// 			tl.removeAll("Activated_On");
// 			tl.removeAll("Activated_Off");
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
	int bd (buildIndex());
	if( bd == 0 )
		return 0;
	
	int cp (copyFiles());
	
	buildHtml();
	
	return cp;
}

int DataExport::buildHtml()
{
	QFile file(exDir.absolutePath() + exDir.separator() +"index.html");
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
	xmlStream.writeStartElement("html");
	
	// header
	xmlStream.writeStartElement("head");
	xmlStream.writeStartElement("title");
	xmlStream.writeCharacters( "Fontmatrix - " + filter);
	xmlStream.writeEndElement();
	xmlStream.writeEndElement();
	
	xmlStream.writeStartElement("body");
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		FontItem* fitem(fonts[fidx]);
		{
			QFileInfo ffile(fitem->path());
			
			xmlStream.writeStartElement("div");
			xmlStream.writeAttribute("class", "fontbox");
			
			xmlStream.writeStartElement("div");
			xmlStream.writeAttribute("class", "namebox");
			
			xmlStream.writeStartElement("a");
			xmlStream.writeAttribute("href", ffile.fileName() );
			xmlStream.writeCharacters( fitem->fancyName() );
			xmlStream.writeEndElement();// a
			
			xmlStream.writeEndElement();// div.namebox
			
			xmlStream.writeStartElement("img");
			xmlStream.writeAttribute("class", "imgbox");
			xmlStream.writeAttribute("src", ffile.fileName() + ".png");
			xmlStream.writeEndElement();// img.imgbox
			
			xmlStream.writeStartElement("div");
			xmlStream.writeAttribute("class", "infobox");
			
			QStringList tl = fitem->tags();
// 			tl.removeAll("Activated_On");
// 			tl.removeAll("Activated_Off");
			foreach(QString tag, tl)
			{
				xmlStream.writeStartElement("div");
				xmlStream.writeAttribute("class", "tagbox");
				xmlStream.writeCharacters( tag );
				xmlStream.writeEndElement(); //div.tagbox
			}
			xmlStream.writeEndElement();// div.info
			xmlStream.writeEndElement();// div.fontbox
		}		
	}
	xmlStream.writeEndElement();// body
	xmlStream.writeEndElement();// html
	xmlStream.writeEndDocument();
	file.close();
	return fonts.count();
}

int DataExport::buildTemplate(const QString& templateDirPath)
{
	/// A template is the addition of 3 files in a directory
	/// Which are : TOP CENTER BOTTOM
	/// In TOP and BOTTOM, just put what you want
	/// In CENTER, these strings will be replaced:
	// ##FILENAME##
	// ##FAMILY##
	// ##VARIANT##
	// ##PREVIEW##
	// ##TAGS##
	
	QFile file(exDir.absolutePath() + exDir.separator() +"export.html");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Export Warning : Can't open " << file.fileName();
		return 0;
	}
	
	QDir tDir(templateDirPath);
	if(!tDir.exists("TOP"))
		return 0;
	if(!tDir.exists("CENTER"))
		return 0;
	if(!tDir.exists("BOTTOM"))
		return 0;
	
	
	QFile fileTOP(tDir.filePath("TOP"));
	QFile fileCENTER(tDir.filePath("CENTER"));
	QFile fileBOTTOM(tDir.filePath("BOTTOM"));
	
	if(!fileTOP.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;
	if(!fileCENTER.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;
	if(!fileBOTTOM.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;
	
	QString sTOP(fileTOP.readAll());
	fileTOP.close();
	QString sCENTER(fileCENTER.readAll());
	fileCENTER.close();
	QString sBOTTOM(fileBOTTOM.readAll());
	fileBOTTOM.close();
	
	QTextStream exp(&file);
	
	exp << sTOP;
	for(int fidx( 0 ); fidx < fonts.count() ; ++fidx)
	{
		FontItem* fitem(fonts[fidx]);
		{
			QString t(sCENTER);
			QFileInfo ffile(fitem->path());
			
			t.replace("##FILENAME##", ffile.fileName() );
			t.replace("##PREVIEW##",  ffile.fileName()+".png");
			t.replace("##FAMILY##", fitem->family() );
			t.replace("##VARIANT##", fitem->variant() );
			t.replace("##TAGS##", fitem->tags().join(", "));
			
			exp << t;
		}
	}
	exp << sBOTTOM;
	
	file.close();	
	return fonts.count();
}


