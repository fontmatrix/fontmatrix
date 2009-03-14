//
// C++ Implementation: fmpdffontextractor
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmpdffontextractor.h"

#include <QDebug>
#include <QFile>



FMPDFFontExtractor::FMPDFFontExtractor()
	:cachedList(false)
{

}

bool FMPDFFontExtractor::loadFile(const QString & filePath)
{
	cachedList = false;
	mfont.clear();
	mType.clear();
	if(document)
		delete document;
	
	if(!QFile::exists(filePath))
	{
		document = 0;
		return false;
	}
	else
	{
		try
		{
			document = new PoDoFo::PdfMemDocument(filePath.toLocal8Bit().data());
		}
		catch(PoDoFo::PdfError& e)
		{
			qDebug()<<"PoDoFo::Error:"<<PoDoFo::PdfError::ErrorMessage( e.GetError() );
			qDebug()<<"Unable to process the PDF file"<<filePath;
			document = 0;
			return false;
		}
	}
	return true;
}

FMPDFFontExtractor::~ FMPDFFontExtractor()
{
	if(document)
		delete document;
}

QStringList FMPDFFontExtractor::extensions()
{
	QStringList ret;
	ret << "pdf" << "PDF";
	return ret;
}

QStringList FMPDFFontExtractor::list()
{
	if(!document)
		return mfont.keys();
	
	if(cachedList)
		return mfont.keys();
	else
		cachedList = true;
	
	PoDoFo::TCIVecObjects objIt( document->GetObjects().begin() );
	PoDoFo::PdfName pType("Type");
	PoDoFo::PdfName pSubtype("Subtype");
	PoDoFo::PdfName pFont("Font");
	PoDoFo::PdfName pType1("Type1");
	PoDoFo::PdfName pTrueType("TrueType");
	PoDoFo::PdfName pFontDescriptor( "FontDescriptor" );
	PoDoFo::PdfName pFontFile( "FontFile" );
	PoDoFo::PdfName pFontFile3( "FontFile3" );
	PoDoFo::PdfName pFontName( "FontName" );
	
	while( objIt != document->GetObjects().end() )
	{
		PoDoFo::PdfObject* obj(*objIt);	
		if ( obj->IsDictionary() )
		{
			if(obj->GetIndirectKey(pType))
			{
				PoDoFo::PdfName type( obj->GetIndirectKey(pType)->GetName() );
				if(type == pFont)
				{
					if(obj->GetIndirectKey( pSubtype ))
					{
						PoDoFo::PdfName subtype ( obj->GetIndirectKey( pSubtype )->GetName() );
						if ((subtype == pType1) ||  (subtype == pTrueType))
						{
							PoDoFo::PdfObject * fontDescriptor ( obj->GetIndirectKey ( pFontDescriptor ) );
							if (fontDescriptor )
							{
								bool hasFile(false);
								PoDoFo::PdfObject * fontFile ( fontDescriptor->GetIndirectKey ( pFontFile ) );
								if ( !fontFile )
								{
									fontFile = fontDescriptor->GetIndirectKey(pFontFile3) ;
									if ( !fontFile )
										qWarning ( "Font not embedded not supported yet" );
									else
										hasFile = true;
			
								}
								else
									hasFile = true;
								if(hasFile)
								{
									PoDoFo::PdfName fontName(fontDescriptor->GetIndirectKey(pFontName)->GetName());
									if(1)
									{
										QString n(QString::fromStdString( fontName.GetName() ));
										mfont[n] = fontFile;
										mType[n] = (subtype == pType1) ? "pfb" : "ttf";
									}
									else
										qDebug()<<"Error: no /FontName key";
								}
							}
		
						}
					}
				}
			}
		}
		objIt++;
	}
	
	return mfont.keys();
	
}

bool FMPDFFontExtractor::write(const QString & name, QIODevice*  openedDevice)
{
	
	if(!mfont.contains(name))
		return false;
	
	PoDoFo::PdfObject * fontFile(mfont[name]);
	
	PoDoFo::PdfMemoryOutputStream outMemStream ( 1 );
	try
	{
		fontFile->GetStream()->GetFilteredCopy ( &outMemStream );
	}
	catch ( PoDoFo::PdfError & e )
	{
		qDebug() <<"Arg, unable to get a filtered copy of a fontfile stream";
		return false;
	}
	outMemStream.Close();

	QByteArray a(outMemStream.TakeBuffer(), outMemStream.GetLength());
	return (openedDevice->write(a) == outMemStream.GetLength());
	
}

QString FMPDFFontExtractor::fontType(const QString & name)
{
	return mType.value(name);
}



