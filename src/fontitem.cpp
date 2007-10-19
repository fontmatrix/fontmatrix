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

#include FT_XFREE86_H
#include FT_GLYPH_H
#include FT_OUTLINE_H 

FT_Library FontItem::theLibrary = 0;
QMap<FT_Encoding, QString> FontItem::charsetMap;

/** functions set for decomposition
 */
static int _moveTo ( const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*>( user );
	p->moveTo ( to->x, to->y );
	return 0;
}
static int _lineTo ( const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*>( user );
	p->lineTo ( to->x, to->y );
	return  0;
}
static int _conicTo ( const FT_Vector* control, const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*>( user );
	p->quadTo ( control->x,control->y,to->x,to->y );
	return 0;
}
static int _cubicTo ( const FT_Vector* control1, const FT_Vector* control2, const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*>( user );
	p->cubicTo ( control1->x,control1->y,control2->x,control2->y,to->x,to->y );
	return 0;
}

FT_Outline_Funcs outline_funcs={
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
	if ( charsetMap.isEmpty() )
		fillCharsetMap();

	allIsRendered = false;

	m_path = path;
	m_name = QFileInfo ( m_path ).fileName();
	if ( ! ensureLibrary() )
		qDebug() << "Unable to init freetype library" ;
	ft_error = FT_New_Face ( theLibrary, path.toLocal8Bit() , 0, &m_face );
	if ( ft_error )
		qDebug() << "Error loading face";

	m_type = FT_Get_X11_Font_Format ( m_face );
	m_family = m_face->family_name;
	m_variant = m_face->style_name;
	m_numGlyphs = m_face->num_glyphs;
	m_numFaces = m_face->num_faces;
	m_faceFlags = testFlag ( m_face->face_flags, FT_FACE_FLAG_SCALABLE, "Is scalable\n","" );
	m_faceFlags += testFlag ( m_face->face_flags, FT_FACE_FLAG_FIXED_WIDTH, "Is monospace\n","" );
	m_faceFlags += testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "Is SFNT based (open or true type)\n","" );
	m_faceFlags += testFlag ( m_face->face_flags, FT_FACE_FLAG_GLYPH_NAMES, "Has glyphs names\n","Has not glyphs names\n" );
	for ( int i = 1;i < m_face->num_charmaps; ++i )
	{
		m_charsets << charsetMap[m_face->charmaps[i]->encoding];
	}

	m_glyph = m_face->glyph;
	m_lock = false;
	pixList.clear();
	sceneList.clear();

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
		return false;
	return true;
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

QGraphicsPathItem * FontItem::itemFromChar(int charcode, double size)
{
	ft_error = FT_Load_Char ( m_face, charcode  , FT_LOAD_NO_SCALE);//spec.at ( i ).unicode()
	if ( ft_error )
	{
		return 0;
	}
	FT_Outline *outline = &m_glyph->outline;
	QPainterPath glyphPath ( QPointF ( 0.0,0.0 ) );
	FT_Outline_Decompose ( outline, &outline_funcs, &glyphPath );
		
	QGraphicsPathItem *glyph = new  QGraphicsPathItem;
	glyph->setBrush(QBrush ( Qt::SolidPattern ) );
	glyph->setPath ( glyphPath );
	double scalefactor = size / m_face->units_per_EM;
	glyph->scale(scalefactor,-scalefactor);
	double advance = m_glyph->metrics.horiAdvance * scalefactor;
return glyph;

}

void FontItem::renderLine ( QGraphicsScene * scene, QString spec, QPointF origine, double fsize)
{
	sceneList.append ( scene );
	double sizz = fsize;
	QPointF pen( origine );
	for ( int i=0; i < spec.length(); ++i )
	{
		QGraphicsPathItem *glyph = itemFromChar(spec.at ( i ).unicode(), sizz);
		glyphList.append(glyph);
		scene->addItem ( glyph);
		glyph->setPos ( pen );
		double scalefactor = sizz / m_face->units_per_EM;
		pen.rx() += m_glyph->metrics.horiAdvance * scalefactor;
	}
}

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
	qDebug() << m_name  <<" ::deRenderAll()";
	for ( int i = 0; i < pixList.count(); ++i )
	{
		pixList[i]->scene()->removeItem ( pixList[i] );
	}
	pixList.clear();
	for ( int i = 0; i < glyphList.count(); ++i )
	{
		glyphList[i]->scene()->removeItem ( glyphList[i] );
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
	for ( uint i = 0 ; i < len; ++i )
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
	if ( allIsRendered )
		return;
	deRender ( scene );

	QPointF pen ( 0,0 );
	int glyph_count = 0;
	int nl = 0;

	for(int i=1;i<=m_numGlyphs; ++i)
		m_charLess.append(i);

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
		ft_error = FT_Load_Glyph ( m_face, gindex , FT_LOAD_RENDER );
		if ( ft_error )
		{
			charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
			continue;
		}

		QByteArray par = pixarray ( m_face->glyph->bitmap.buffer, m_face->glyph->bitmap.width * m_face->glyph->bitmap.rows );

		QImage image ( reinterpret_cast<uchar*> ( par.data() ), m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows,  QImage::Format_ARGB32 );

		
		QGraphicsPixmapItem *pitem = new QGraphicsPixmapItem ( QPixmap::fromImage ( image ) );
		pitem->setShapeMode ( QGraphicsPixmapItem::BoundingRectShape );
		pitem->setFlag ( QGraphicsItem::ItemIsSelectable,true );
		pitem->setData ( 1,gindex );
		uint ucharcode = charcode;
		pitem->setData ( 2,ucharcode );
		pixList.append ( pitem );
		scene->addItem ( pitem );
		pitem->setPos ( pen );
		pitem->moveBy ( m_face->glyph->bitmap_left , - m_face->glyph->bitmap_top );
		pen.rx() += 100;

		++glyph_count;
		m_charLess.removeAll(gindex);
		++nl;
		charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
	}
	
	// We want OpenTyped glyphs
	nl = 0;
	pen.rx() = 0;
	pen.ry() += 100;
	for(int gi=0;gi<m_charLess.count(); ++gi)
	{
		if ( nl == 6 )
		{
			nl = 0;
			pen.rx() = 0;
			pen.ry() += 100;
		}
		ft_error = FT_Load_Glyph ( m_face, m_charLess[gi], FT_LOAD_RENDER );
		if ( ft_error )
		{
			
			continue;
		}

		QByteArray par = pixarray ( m_face->glyph->bitmap.buffer, m_face->glyph->bitmap.width * m_face->glyph->bitmap.rows );

		QImage image ( reinterpret_cast<uchar*> ( par.data() ), m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows,  QImage::Format_ARGB32 );

		image.invertPixels(QImage::InvertRgba);
		QGraphicsPixmapItem *pitem = new QGraphicsPixmapItem ( QPixmap::fromImage ( image ) );
		pitem->setShapeMode ( QGraphicsPixmapItem::BoundingRectShape );
		pitem->setFlag ( QGraphicsItem::ItemIsSelectable,true );
		pitem->setData ( 1,gindex );
		uint ucharcode = charcode;
		pitem->setData ( 2,0 );
		pixList.append ( pitem );
		scene->addItem ( pitem );
		pitem->setPos ( pen );
		pitem->moveBy ( m_face->glyph->bitmap_left , - m_face->glyph->bitmap_top );
		pen.rx() += 100;
		++nl;
	}
	
	qDebug() << m_name <<m_charLess.count();
	allIsRendered = true;
}

QString FontItem::infoText()
{
	QString ret ( "<h3>%1</h3> <p>%2</p> <p>%3</p> <p>%4</p>  <p>%5 glyphs in %6 faces (including %8 glyphs unreachable by character codes)</p><p>%7</p>" );
	return ret.arg ( m_family + " " + m_variant )//1
	       .arg ( m_path )//2
	       .arg ( m_type )//3
	       .arg ( m_charsets.join ( ", " ) )//4
	       .arg ( m_numGlyphs )//5
	       .arg ( m_numFaces )//6
	       .arg ( m_tags.join ( ", " ) )//7
	       .arg(m_charLess.count() - 1)//8
			;
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

	QString ret ( "<h3>%1 \t(from %7)</h3> U+%6 at index %2<p> width = %3, height = %4, advance = %5</p>" );
	ft_error = FT_Load_Glyph ( m_face, index , FT_LOAD_NO_SCALE );
	if ( ft_error )
	{
		ret = "%1 is broken";
		return ret.arg ( QChar ( code ) );
	}
	return ret.arg ( key )
	       .arg ( index )
	       .arg ( m_glyph->metrics.width )
	       .arg ( m_glyph->metrics.height )
	       .arg ( m_glyph->metrics.horiAdvance )
	       .arg ( code, 4, 16,QLatin1Char ( '0' ) )
	       .arg ( m_name );
}




