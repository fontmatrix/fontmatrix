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
#include "fontitem.h"

#include <QDebug>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>

#include FT_XFREE86_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TYPE1_TABLES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H 


FT_Library FontItem::theLibrary = 0;
QGraphicsScene *FontItem::theOneLineScene = 0;
QMap<FT_Encoding, QString> FontItem::charsetMap;

/** functions set for decomposition
 */
static int _moveTo ( const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->moveTo ( to->x, to->y );
	return 0;
}
static int _lineTo ( const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->lineTo ( to->x, to->y );
	return  0;
}
static int _conicTo ( const FT_Vector* control, const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->quadTo ( control->x,control->y,to->x,to->y );
	return 0;
}
static int _cubicTo ( const FT_Vector* control1, const FT_Vector* control2, const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->cubicTo ( control1->x,control1->y,control2->x,control2->y,to->x,to->y );
	return 0;
}

FT_Outline_Funcs outline_funcs=
{
	_moveTo,
	_lineTo,
	_conicTo,
	_cubicTo,
	0,
	0
};
/** **************************************************/


void fillCharsetMap()
{
	FontItem::charsetMap[FT_ENCODING_UNICODE] = "UNICODE ";

	FontItem::charsetMap[FT_ENCODING_MS_SYMBOL] = "MS_SYMBOL";
	FontItem::charsetMap[FT_ENCODING_SJIS] = "SJIS .";
	FontItem::charsetMap[FT_ENCODING_GB2312	] = "GB2312 ";
	FontItem::charsetMap[FT_ENCODING_BIG5] = "BIG5 ";
	FontItem::charsetMap[FT_ENCODING_WANSUNG] = "WANSUNG ";
	FontItem::charsetMap[FT_ENCODING_JOHAB] = "JOHAB ";
	FontItem::charsetMap[FT_ENCODING_ADOBE_LATIN_1] = "ADOBE_LATIN_1 ";
	FontItem::charsetMap[FT_ENCODING_ADOBE_STANDARD] = "ADOBE_STANDARD ";
	FontItem::charsetMap[FT_ENCODING_ADOBE_EXPERT] = "ADOBE_EXPERT ";
	FontItem::charsetMap[FT_ENCODING_ADOBE_CUSTOM] = "ADOBE_CUSTOM ";
	FontItem::charsetMap[FT_ENCODING_APPLE_ROMAN] = "APPLE_ROMAN ";
	FontItem::charsetMap[FT_ENCODING_OLD_LATIN_2] = "This value is deprecated and was never used nor reported by FreeType. Don't use or test for it.";
	FontItem::charsetMap[FT_ENCODING_MS_SJIS] = "MS_SJIS ";
	FontItem::charsetMap[FT_ENCODING_MS_GB2312] = "MS_GB2312 ";
	FontItem::charsetMap[FT_ENCODING_MS_BIG5] = "MS_BIG5 ";
	FontItem::charsetMap[FT_ENCODING_MS_WANSUNG] = "MS_WANSUNG ";
	FontItem::charsetMap[FT_ENCODING_MS_JOHAB] = "MS_JOHAB ";
}


		
FontItem::FontItem ( QString path )
{
// 	qDebug() << path;
	
	m_face = 0;
	
	if ( charsetMap.isEmpty() )
		fillCharsetMap();
	if(!theOneLineScene)
	{
		theOneLineScene = new QGraphicsScene(0,0,320,32);
	}

	allIsRendered = false;

	m_path = path;
	QFileInfo infopath( m_path );
	m_name = infopath.fileName();
	
	if ( ! ensureFace())
	{
		
		return;
	}
	
	if(infopath.suffix() == "pfb")
	{
		if(!ft_error)
		{
			m_afm = m_path;
			m_afm.replace(".pfb",".afm");
			ft_error = FT_Attach_File(m_face, m_afm.toLocal8Bit());
			if(ft_error)
				m_afm ="";
		}
	}

	m_type = FT_Get_X11_Font_Format ( m_face );
	m_family = m_face->family_name;
	m_variant = m_face->style_name;
	m_numGlyphs = m_face->num_glyphs;
	m_numFaces = m_face->num_faces;
	
	for ( int i = 1;i < m_face->num_charmaps; ++i )
	{
		m_charsets << charsetMap[m_face->charmaps[i]->encoding];
	}

	
	m_lock = false;
	pixList.clear();
	sceneList.clear();
	
	//fill cache and avoid a further call to ensureface
	infoText();
	
	releaseFace();

}


FontItem::~FontItem()
{
}

bool FontItem::ensureLibrary()
{
	if ( theLibrary )
		return true;
	ft_error = FT_Init_FreeType ( &theLibrary );
	if ( ft_error )
	{
		qDebug() << "Error loading ft_library ";
		return false;
	}
	return true;
}

bool FontItem::ensureFace()
{
// 	qDebug("ENSUREFACE") ;
	if(ensureLibrary())
	{
		if(m_face)
			return true;
		ft_error = FT_New_Face ( theLibrary, m_path.toLocal8Bit() , 0, &m_face );
		if ( ft_error )
		{
			qDebug() << "Error loading face [" << m_path <<"]";
			return false;
		}
		m_glyph = m_face->glyph;
		return true;
	}
	return false;
}

void FontItem::releaseFace()
{
// 	qDebug("\t\tRELEASEFACE") ;
	if(m_face)
	{
		FT_Done_Face(m_face);
		m_face = 0;
	}
}


QString FontItem::testFlag ( long flag, long against, QString yes, QString no )
{
	if ( ( flag & against ) == against )
		return yes;
	else
		return no;
}

QString FontItem::value ( QString k )
{
	if ( k == "family" )
		return m_family;
	if ( k == "variant" )
		return m_variant;

	//default
	return QString();

}

QString FontItem::name()
{
	return m_name;
}

QGraphicsPathItem * FontItem::itemFromChar ( int charcode, double size )
{
	
	
	if ( !contourCache.contains ( charcode ) )
	{
		ft_error = FT_Load_Char ( m_face, charcode  , FT_LOAD_NO_SCALE );//spec.at ( i ).unicode()
		if ( ft_error )
		{
			return 0;
		}
// 		FT_Outline *outline = &m_glyph->outline;
		QPainterPath glyphPath ( QPointF ( 0.0,0.0 ) );
		FT_Outline_Decompose ( &m_glyph->outline, &outline_funcs, &glyphPath );
		contourCache[charcode] = glyphPath;
		advanceCache[charcode] =  m_glyph->metrics.horiAdvance;
	}

	QGraphicsPathItem *glyph = new  QGraphicsPathItem;
	glyph->setBrush ( QBrush ( Qt::SolidPattern ) );
	glyph->setPath ( contourCache[charcode] );
	double scalefactor = size / m_face->units_per_EM;
	glyph->scale ( scalefactor,-scalefactor );
	return glyph;
	
	

}

QGraphicsPathItem * FontItem::itemFromGindex ( int index, double size )
{
	int charcode = index + 65536;
	if ( !contourCache.contains ( charcode ) )
	{
		ft_error = FT_Load_Glyph ( m_face, charcode  - 65536, FT_LOAD_NO_SCALE );//spec.at ( i ).unicode()
		if ( ft_error )
		{
			return 0;
		}
		FT_Outline *outline = &m_glyph->outline;
		QPainterPath glyphPath ( QPointF ( 0.0,0.0 ) );
		FT_Outline_Decompose ( outline, &outline_funcs, &glyphPath );
		contourCache[charcode] = glyphPath;
		advanceCache[charcode] =  m_glyph->metrics.horiAdvance;
	}

	QGraphicsPathItem *glyph = new  QGraphicsPathItem;
	glyph->setBrush ( QBrush ( Qt::SolidPattern ) );
	glyph->setPath ( contourCache[charcode] );
	double scalefactor = size / m_face->units_per_EM;
	glyph->scale ( scalefactor,-scalefactor );
	return glyph;
}


void FontItem::renderLine ( QGraphicsScene * scene, QString spec, QPointF origine, double fsize ,bool record)
{
	ensureFace();
	if(record)
		sceneList.append ( scene );
	double sizz = fsize;
	QPointF pen ( origine );
	for ( int i=0; i < spec.length(); ++i )
	{
		QGraphicsPathItem *glyph = itemFromChar ( spec.at ( i ).unicode(), sizz );
		if(record)
			glyphList.append ( glyph );
		scene->addItem ( glyph );
		glyph->setPos ( pen );
		glyph->setZValue ( 100.0 );
		glyph->setData(1,"glyph");
		double scalefactor = sizz / m_face->units_per_EM;
		pen.rx() += advanceCache[spec.at ( i ).unicode() ] * scalefactor;
	}
	
	releaseFace();
}

//deprecated
void FontItem::deRender ( QGraphicsScene *scene )
{
	QList<int> rem;
	for ( int i = 0; i < pixList.count(); ++i )
	{
		if ( pixList[i]->scene() == scene )
		{
			scene->removeItem ( pixList[i] );
			rem.append ( i );
		}
	}
	for ( int i = rem.count() - 1; i >= 0; --i )
		pixList.removeAt ( rem[i] );
}

void FontItem::deRenderAll()
{
	qDebug() << m_name  <<"::deRenderAll()";
	for ( int i = 0; i < pixList.count(); ++i )
	{
		if ( pixList[i]->scene() )
		{
			pixList[i]->scene()->removeItem ( pixList[i] );
			delete pixList[i];
		}
	}
	pixList.clear();
	for ( int i = 0; i < glyphList.count(); ++i )
	{
		if ( glyphList[i]->scene() )
		{
			glyphList[i]->scene()->removeItem ( glyphList[i] );
			delete glyphList[i];
		}
	}
	glyphList.clear();

	allIsRendered = false;
}

QByteArray FontItem::pixarray ( uchar * b, int len )
{
// 	QByteArray ar(len * 4);
// 	for(i = 0; i<len;)
// 		ar[i] = qRgb(b[i],b[i],b[i]);
// 	return ar;
//
	uchar *imgdata =  b ;
	QByteArray buffer ( len * 4, 255 );
	QDataStream stream ( &buffer,QIODevice::WriteOnly );
	for ( int i = 0 ; i < len; ++i )
	{

		stream << ( quint8 ) ~imgdata[i];
		stream << ( quint8 ) ~imgdata[i];
		stream << ( quint8 ) ~imgdata[i];
		stream << ( quint8 ) imgdata[i];
	}

	return buffer;
}

void FontItem::renderAll ( QGraphicsScene * scene )
{
	ensureFace();
	
	if ( allIsRendered )
		return;
	deRender ( scene );

	QPointF pen ( 0,0 );
	int glyph_count = 0;
	int nl = 0;

	for ( int i=1;i<=m_numGlyphs; ++i )
		m_charLess.append ( i );

	FT_ULong  charcode;
	FT_UInt   gindex;
	double sizz = 50;
	FT_Set_Char_Size ( m_face, sizz * 64 , 0, QApplication::desktop()->logicalDpiX(), QApplication::desktop()->logicalDpiY() );

	charcode = FT_Get_First_Char ( m_face, &gindex );
	while ( gindex != 0 )
	{
		if ( nl == 6 )
		{
			nl = 0;
			pen.rx() = 0;
			pen.ry() += 100;
		}
		QGraphicsPathItem *pitem = itemFromChar ( charcode , sizz );
		pitem->setFlag ( QGraphicsItem::ItemIsSelectable,true );
		pitem->setData ( 1,gindex );
		uint ucharcode = charcode;
		pitem->setData ( 2,ucharcode );
		glyphList.append ( pitem );
		scene->addItem ( pitem );
		pitem->setPos ( pen );
		pen.rx() += 100;

		++glyph_count;
		m_charLess.removeAll ( gindex );
		++nl;
		charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
	}

	// We want featured glyphs
	nl = 0;
	pen.rx() = 0;
	pen.ry() += 100;
	for ( int gi=0;gi<m_charLess.count(); ++gi )
	{
		if ( nl == 6 )
		{
			nl = 0;
			pen.rx() = 0;
			pen.ry() += 100;
		}
		QGraphicsPathItem *pitem = itemFromGindex ( m_charLess[gi], sizz );
		if ( !pitem )
		{
			qDebug() << "pitem is null for m_charLess[gi] = "  << m_charLess[gi];
			continue;
		}
		pitem->setFlag ( QGraphicsItem::ItemIsSelectable,true );
		pitem->setData ( 1,m_charLess[gi] );
// 		uint ucharcode = charcode;
		pitem->setData ( 2,0 );
		glyphList.append ( pitem );
		scene->addItem ( pitem );
		pitem->setPos ( pen );
		pen.rx() += 100;
		++nl;
	}

	qDebug() << m_name <<m_charLess.count();
	allIsRendered = true;
	
	releaseFace();
}

QString FontItem::infoText()
{
	if(!m_cacheInfo.isEmpty())
		return m_cacheInfo;
	
	ensureFace();

	QString ret("<h2 style=\"color:white;background-color:black;\">" + fancyName() + "</h2>\n");
	ret += "<p>"+ QString::number(m_numGlyphs) + " glyphs; " + m_charsets.join ( ", " )+"</p>";
	ret += "<p style=\"background-color:#aaa;\"><b>Tags  </b>"+ m_tags.join ( ", " ) +"</p>";
// 	Some place to add things
// 	ret += "<p>"+  +"</p>";
// 	ret += "<p>"+  +"</p>";
// 	ret += "<p>"+  +"</p>";
	if(moreInfo.isEmpty())
	{
		if( testFlag(m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0") == "1")
		{
			moreInfo_sfnt();
		}
		if(m_path.endsWith(".pfb",Qt::CaseInsensitive ))
		{
			moreInfo_type1();
// 			qDebug() << "TYPE1";
		}
	}
	if(moreInfo.count())
	{
// 		ret += "<p> \n\t- Extra Info -</p>";
		for(QMap<QString, QString>::const_iterator mit = moreInfo.begin(); mit != moreInfo.end();++mit)
		{
			ret += "<p><b>"   + mit.key() + " </b> " + mit.value() + "</p>";
		};
	}
	m_cacheInfo = ret;
	
	releaseFace();
	return ret;
}


QString FontItem::infoGlyph ( int index, int code )
{
	QString key;
	if ( FT_HAS_GLYPH_NAMES ( m_face ) )
	{
		char buf[256];
		FT_Get_Glyph_Name ( m_face, index, buf, 255 );
		if ( buf[0] != 0 )
		{
			key = buf;
		}
		else
		{
			key = "noname" + index;
		}
	}
	else
	{
		key = "noname" + index;
	}

	QString ret ( "%1 \t(from %2), U+%3 at index %4" );
	return ret.arg ( key )
	       .arg ( m_name )
	       .arg ( code, 4, 16,QLatin1Char ( '0' ) )
	       .arg ( index )
	       ;
}

//deprecated
QString FontItem::toElement()
{
	QString ret;
	ret = "<fontfile><file>%1</file><tag>%2</tag></fontfile>";
	return ret.arg(name()).arg(tags().join("</tag><tag>"));
}

QGraphicsPathItem * FontItem::hasCodepoint(int code)
{
	for(int i=0;i< glyphList.count();++i)
	{
		if(glyphList.at(i)->data(2).toInt() == code)
			return glyphList.at(i);
	}
	return 0;
}

QIcon  FontItem::oneLinePreviewIcon()
{
	if(!theOneLinePreviewIcon.isNull())
		return theOneLinePreviewIcon;
	
	theOneLineScene->removeItem(theOneLineScene->createItemGroup(theOneLineScene->items()));
	renderLine(theOneLineScene,"a",QPointF(0,32),50,false);
	QPixmap apix(32,32);
	apix.fill(Qt::white);
	QPainter apainter(&apix);
	apainter.setRenderHint(QPainter::Antialiasing,true);
	theOneLineScene->render(&apainter,apix.rect(),apix.rect());
// 	theOneLinePreviewIcon.addPixmap(apix);
	theOneLinePreviewIcon = apix;
	return theOneLinePreviewIcon;
}

QPixmap FontItem::oneLinePreviewPixmap()
{
	if(!theOneLinePreviewPixmap.isNull())
		return theOneLinePreviewPixmap;
	
	theOneLineScene->removeItem(theOneLineScene->createItemGroup(theOneLineScene->items()));
	renderLine(theOneLineScene,fancyName(),QPointF(0,16),20,false);
	QPixmap apix(50 * 32,32);
	apix.fill(Qt::white);
	QPainter apainter(&apix);
	apainter.setRenderHint(QPainter::Antialiasing,true);
	theOneLineScene->render(&apainter);
	theOneLinePreviewPixmap = apix;
	return theOneLinePreviewPixmap;
}

/** reminder
FT_SfntName::name_id
Code  	Meaning
0 	Copyright 
1 	Font Family 
2 	Font Subfamily
3 	Unique font identifier
4 	Full font name
5 	Version string
6 	Postscript name for the font
7 	Trademark
8 	Manufacturer Name.
9 	Designer
10 	Description
11 	URL Vendor
12 	URL Designer
13 	License Description
14 	License Info URL
15 	Reserved; Set to zero.
16 	Preferred Family
17 	Preferred Subfamily
18 	Compatible Full (Macintosh only)
19 	Sample text
20 	PostScript CID findfont name
*/
void FontItem::moreInfo_sfnt()
{
	FT_SfntName tname;
	static QStringList name_meaning;
	if(name_meaning.isEmpty())
	{
		name_meaning << "Copyright"
				<< "Font Family"
				<< "Font Subfamily"
				<< "Unique font identifier"
				<< "Full font name"
				<< "Version string"
				<< "Postscript name"
				<< "Trademark"
				<< "Manufacturer"
				<< "Designer"
				<< "Description"
				<< "URL Vendor"
				<< "URL Designer"
				<< "License Description"
				<< "License Info URL"
				<< "Reserved"
				<< "Preferred Family"
				<< "Preferred Subfamily"
				<< "Compatible Full (Macintosh only)"
				<< "Sample text"
				<< "PostScript CID findfont name";
	}
	int tname_count = FT_Get_Sfnt_Name_Count(m_face);
	//TODO check encodings and platforms
	for(int i=0; i < tname_count; ++i)
	{
		FT_Get_Sfnt_Name(m_face,i,&tname);
		QString akey;
		if(tname.name_id >  255)
		{
// 			qDebug() << name() <<" has vendor’s specific name id ->" << tname.name_id;
			if(tname.string_len > 0)
			{
				akey = "VendorKey_" + QString::number(tname.name_id);
			}
			else
			{
				continue;
			}
			
		}
		else if(tname.name_id <= name_meaning.count())
		{
			akey = name_meaning.at(tname.name_id);
		}
		else
		{
			qDebug() << "It seems there are new name IDs in TT spec, please say FontMatrix’s team to stay up to dat.";
			continue;
		}
		
		if(!moreInfo.contains(akey) )
		{
			QString avalue;
			if(tname.platform_id == TT_PLATFORM_APPLE_UNICODE)//just catch for the moment
			{
				qDebug() << "Platform id is TT_PLATFORM_APPLE_UNICODE";
			}
			else if(tname.platform_id == TT_PLATFORM_MICROSOFT)//freetype reports it is the most used, we than focus on it at first
			{
				if(tname.encoding_id == TT_MS_ID_SYMBOL_CS )
				{
					qDebug() << "encoding is symbol";
				}
				else if(tname.encoding_id == TT_MS_ID_UNICODE_CS )
				{
// 					QList<QChar> tstring;
					for(int c=0; c < tname.string_len; ++c)
					{
						avalue.append( QChar(tname.string[c]) );
					}
// 					QByteArray tar((const char *)tname.string, ); 
// 					value = QString::fromUtf16(tar.data());
				}
				else
				{
					qDebug() << "Not handled encoding";
				}
			}
			else if(TT_PLATFORM_MACINTOSH == tname.platform_id)
			{
				
				QByteArray anar((char*)tname.string, tname.string_len);
				avalue = anar;
// 				qDebug() << "TT_PLATFORM_MACINTOSH\n" << "\t"<< tname.string_len <<"\t"<<avalue;
			}
			else
			{
				qDebug() << "Platform id ("<< tname.platform_id <<")is unhandled now";
			}
			if(!avalue.isEmpty())
			{
// 				qDebug()<<akey ;
				moreInfo[akey] = avalue;
// 				qDebug()<< "\t\t"<<avalue;
			}
		}		
	}
}

void FontItem::moreInfo_type1()
{
	PS_FontInfoRec sinfo ;
	int err = FT_Get_PS_Font_Info(m_face,&sinfo);
	if(err)
	{
		qDebug() <<"FT_Get_PS_Font_Info("<< m_name <<")"<<" failed :" << err;
		return;
	}
	// full_name version notice
	moreInfo["full_name"] = sinfo.full_name;
	moreInfo["version"] = sinfo.version;
	moreInfo["notice"] = sinfo.notice;
	
}

///return size of dynamic structures

int FontItem::debug_size()
{
	int ret=0;
	for(QMap<int,QPainterPath>::const_iterator cit = contourCache.begin(); cit != contourCache.end();++cit)
		ret+=cit->elementCount();
	
}









