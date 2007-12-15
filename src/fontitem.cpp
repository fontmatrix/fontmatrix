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
#include "fmotf.h"
#include "fmshaper.h"

#include <QDebug>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QLocale>
#include <QTextCodec>



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
QMap<int, QString> FontItem::langIdMap;

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
	FontItem::charsetMap[FT_ENCODING_NONE] = "NONE";
	FontItem::charsetMap[FT_ENCODING_UNICODE] = "UNICODE";

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

// sfnt names
static QStringList name_meaning;

FontItem::FontItem ( QString path )
{
// 	qDebug() << path;

	m_face = 0;
	facesRef = 0;
	m_glyphsPerRow = 5;
	hasUnicode = false;
	currentChar = -1;
	m_isOpenType = false;
	m_rasterFreetype = false;
	fillLangIdMap();

	if ( charsetMap.isEmpty() )
		fillCharsetMap();
	if ( !theOneLineScene )
	{
		theOneLineScene = new QGraphicsScene;
	}

	allIsRendered = false;

	m_path = path;
	QFileInfo infopath ( m_path );
	m_name = infopath.fileName();

	if ( ! ensureFace() )
	{

		return;
	}

	if ( infopath.suffix() == "pfb" )
	{
		if ( !ft_error )
		{
			m_afm = m_path;
			m_afm.replace ( ".pfb",".afm" );
			ft_error = FT_Attach_File ( m_face, m_afm.toLocal8Bit() );
			if ( ft_error )
				m_afm ="";
		}
	}
// 	if ( infopath.suffix() == "otf" ||  infopath.suffix() == "OTF")
// 	{
// 		m_isOpenType = true; // A bit rough, perhaps!
// 	}

	m_type = FT_Get_X11_Font_Format ( m_face );
	m_family = m_face->family_name;
	m_variant = m_face->style_name;
	m_numGlyphs = m_face->num_glyphs;
	m_numFaces = m_face->num_faces;

	for ( int i = 0 ;i < m_face->num_charmaps; ++i )
	{
		m_charsets << charsetMap[m_face->charmaps[i]->encoding];
	}

	m_charsets = m_charsets.toSet().toList();


	m_lock = false;
	pixList.clear();
	sceneList.clear();

	//fill cache and avoid a further call to ensureface
	infoText();

	releaseFace();

}


FontItem::~FontItem()
{
	if ( m_isOpenType )
	{
		delete otf;
	}
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
	if ( ensureLibrary() )
	{
		if ( m_face )
		{
			++facesRef;
			return true;
		}
		ft_error = FT_New_Face ( theLibrary, m_path.toLocal8Bit() , 0, &m_face );
		if ( ft_error )
		{
			qDebug() << "Error loading face [" << m_path <<"]";
			return false;
		}
		ft_error = FT_Select_Charmap ( m_face, FT_ENCODING_UNICODE );
		if ( ft_error )
		{
			hasUnicode = false;
		}
		else
		{
			hasUnicode = true;
		}
		m_glyph = m_face->glyph;
		++facesRef;
		return true;
	}
	return false;
}

void FontItem::releaseFace()
{
// 	qDebug("\t\tRELEASEFACE") ;
	if ( m_face )
	{
		--facesRef;
		if ( facesRef == 0 )
		{
			FT_Done_Face ( m_face );
			m_face = 0;
		}
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

	uint glyphIndex = 0;
	currentChar = charcode;
	glyphIndex = FT_Get_Char_Index ( m_face, charcode );
	if ( glyphIndex == 0 )
	{
		return 0;
	}
	else
	{
		return itemFromGindex ( glyphIndex,size );
	}
	return 0;
}

QGraphicsPathItem * FontItem::itemFromGindex ( int index, double size )
{
	int charcode = index ;
	ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_NO_SCALE );
	if ( ft_error )
	{
		return 0;
	}
	FT_Outline *outline = &m_glyph->outline;
	QPainterPath glyphPath ( QPointF ( 0.0,0.0 ) );
	FT_Outline_Decompose ( outline, &outline_funcs, &glyphPath );
	advanceCache[currentChar] =  m_glyph->metrics.horiAdvance;
	QGraphicsPathItem *glyph = new  QGraphicsPathItem;
	glyph->setBrush ( QBrush ( Qt::SolidPattern ) );
	glyph->setPath ( glyphPath );
	double scalefactor = size / m_face->units_per_EM;
	glyph->scale ( scalefactor,-scalefactor );
	return glyph;
}

QGraphicsPixmapItem * FontItem::itemFromCharPix ( int charcode, double size )
{
	uint glyphIndex = 0;
	currentChar = charcode;
	glyphIndex = FT_Get_Char_Index ( m_face, charcode );
	if ( glyphIndex == 0 )
	{
		return 0;
	}
	else
	{
		return itemFromGindexPix ( glyphIndex,size );
	}
	return 0;
}

QGraphicsPixmapItem * FontItem::itemFromGindexPix ( int index, double size )
{
	int charcode = index ;
	ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_DEFAULT );
	if ( ft_error )
	{
		return 0;
	}
	ft_error = FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL );
	if ( ft_error )
	{
		return 0;
	}

	QVector<QRgb> palette;
	for ( int i = 0; i < m_face->glyph->bitmap.num_grays; ++i )
	{
		palette << qRgba ( 0,0,0, i );
	}
	QImage img ( m_face->glyph->bitmap.buffer,
	             m_face->glyph->bitmap.width,
	             m_face->glyph->bitmap.rows,
	             m_face->glyph->bitmap.pitch,
	             QImage::Format_Indexed8 );
	img.setColorTable ( palette );

	advanceCache[currentChar] =  m_glyph->advance.x >> 6;
	QGraphicsPixmapItem *glyph = new  QGraphicsPixmapItem;
	glyph->setPixmap ( QPixmap::fromImage ( img ) );
	// we need to transport more data
	glyph->setData ( 1,m_face->glyph->bitmap_left );
	glyph->setData ( 2,m_face->glyph->bitmap_top );
	glyph->setData ( 3, ( uint ) m_glyph->advance.x >> 6 );
	return glyph;
}


/// Nature line
void FontItem::renderLine ( QGraphicsScene * scene, QString spec, QPointF origine, double fsize ,bool record )
{
	ensureFace();
	FT_Set_Char_Size ( m_face, fsize  * 64 , 0,72,72 );
	if ( record )
		sceneList.append ( scene );
	double sizz = fsize;
	QPointF pen ( origine );
	if ( m_rasterFreetype )
	{
		for ( int i=0; i < spec.length(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromCharPix ( spec.at ( i ).unicode(), sizz );
			if ( !glyph )
			{
				qDebug() << "Unable to render "<< spec.at ( i ) <<" from "<< name() ;
				continue;
			}
			if ( record )
				pixList.append ( glyph );
			scene->addItem ( glyph );
			glyph->setPos ( pen.x() + glyph->data ( 1 ).toInt(),
			                pen.y() - glyph->data ( 2 ).toInt() );
			glyph->setZValue ( 100.0 );
			glyph->setData ( 1,"glyph" );
			pen.rx() += glyph->data ( 3 ).toInt();
		}
	}
	else
	{
		for ( int i=0; i < spec.length(); ++i )
		{
			QGraphicsPathItem *glyph = itemFromChar ( spec.at ( i ).unicode(), sizz );
			if ( !glyph )
			{
				qDebug() << "Unable to render "<< spec.at ( i ) <<" from "<< name() ;
				continue;
			}
			if ( record )
				glyphList.append ( glyph );
			scene->addItem ( glyph );
			glyph->setPos ( pen );
			glyph->setZValue ( 100.0 );
			glyph->setData ( 1,"glyph" );
			double scalefactor = sizz / m_face->units_per_EM;
			if ( !m_RTL )
				pen.rx() += advanceCache[spec.at ( i ).unicode() ] * scalefactor;
			else
				pen.rx() -= advanceCache[spec.at ( i ).unicode() ] * scalefactor;
		}
	}

	releaseFace();
}

/// Featured line
void FontItem::renderLine ( OTFSet set, QGraphicsScene * scene, QString spec, QPointF origine, double fsize, bool record )
{
	if ( !m_isOpenType )
		return;
	ensureFace();

	otf = new FmOtf ( m_face );
	if ( !otf )
		return;
	if ( record )
		sceneList.append ( scene );
	double sizz = fsize;
	FT_Set_Char_Size ( m_face, sizz  * 64 , 0, 72, 72 );
	QList<RenderedGlyph> refGlyph = otf->procstring ( spec, set );
	if ( refGlyph.count() == 0 )
	{
		return;
	}
	QPointF pen ( origine );

	if ( m_rasterFreetype )
	{
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromGindexPix ( refGlyph[i].glyph , sizz );
			if ( !glyph )
			{
				qDebug() << "Unable to render "<< spec.at ( i ) <<" from "<< name() ;
				continue;
			}
			if ( record )
				pixList.append ( glyph );
			scene->addItem ( glyph );
			double scalefactor = sizz / m_face->units_per_EM  ;
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset  * scalefactor ) + glyph->data ( 1 ).toInt()  ,
			                pen.y() + ( refGlyph[i].yoffset  * scalefactor ) - glyph->data ( 2 ).toInt() );
			glyph->setZValue ( 100.0 );
			glyph->setData ( 1,"glyph" );
			pen.rx() += refGlyph[i].xadvance * scalefactor ;//We’ll have some "rounded" related wrong display but...
		}
	}
	else
	{
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPathItem *glyph = itemFromGindex ( refGlyph[i].glyph , sizz );
			if ( !glyph )
			{
				qDebug() << "Unable to render "<< spec.at ( i ) <<" from "<< name() ;
				continue;
			}
			if ( record )
				glyphList.append ( glyph );
			scene->addItem ( glyph );
			double scalefactor = sizz / m_face->units_per_EM;
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * scalefactor ),
			                pen.y() + ( refGlyph[i].yoffset * scalefactor ) );
			glyph->setZValue ( 100.0 );
			glyph->setData ( 1,"glyph" );
			pen.rx() += refGlyph[i].xadvance * scalefactor;
		}
	}

	delete otf;
	releaseFace();
}

/// Shaped line
void FontItem::renderLine ( QString script, QGraphicsScene * scene, QString spec, QPointF origine, double fsize, bool record )
{
	if ( !m_isOpenType )
		return;
	ensureFace();

	double sizz = fsize;
	double scalefactor = sizz / m_face->units_per_EM;
	qDebug() << scalefactor;

	otf = new FmOtf ( m_face );
	if ( !otf )
	{
		releaseFace();
		return;
	}
	if ( record )
		sceneList.append ( scene );

	FT_Set_Char_Size ( m_face, sizz  * 64 , 0, 72, 72 );

	FmShaper *shaper = new FmShaper;
	if ( !shaper )
	{
		delete otf;
		releaseFace();
		return;
	}
	shaper->setFont ( m_face ,&otf->hbFont );
	if ( !shaper->setScript ( script ) )
	{
		qDebug() << "Can not set script "<<script<< " for " << m_name;
		delete otf;
		releaseFace();
		return;
	}

	QList<RenderedGlyph> refGlyph = shaper->doShape ( spec, !m_RTL );
	delete shaper;

	if ( refGlyph.count() == 0 )
	{
		releaseFace();
		return;
	}
	QPointF pen ( origine );
	if ( m_rasterFreetype )
	{
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromGindexPix ( refGlyph[i].glyph , sizz );
			if ( !glyph )
			{
				qDebug() << "Unable to render "<< spec.at ( i ) <<" from "<< name() ;
				continue;
			}
			if ( record )
				pixList.append ( glyph );
			if ( m_RTL )
				pen.rx() += refGlyph[i].xadvance * scalefactor ;
			scene->addItem ( glyph );
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset  * scalefactor ) + glyph->data ( 1 ).toInt()  ,
			                pen.y() + ( refGlyph[i].yoffset  * scalefactor ) - glyph->data ( 2 ).toInt() );
			glyph->setZValue ( 100.0 );
			glyph->setData ( 1,"glyph" );

			if ( !m_RTL )
				pen.rx() += refGlyph[i].xadvance * scalefactor;
		}
	}
	else
	{
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPathItem *glyph = itemFromGindex ( refGlyph[i].glyph , sizz );
			if ( !glyph )
			{
				qDebug() << "Unable to render "<< refGlyph[i].glyph <<" from "<< name() ;
				continue;
			}
			if ( record )
				glyphList.append ( glyph );
			scene->addItem ( glyph );
			glyph->setZValue ( 100.0 );
			glyph->setData ( 1,"glyph" );
			//debug
			glyph->setBrush ( QColor ( ( i*255/refGlyph.count() ),0,0,255- ( i*255/refGlyph.count() ) ) );
			qDebug() << refGlyph[i].dump();
			qDebug() << "Scaled advance = " << refGlyph[i].xadvance * scalefactor;
			qDebug() << pen;
			//
			if ( m_RTL )
			{
				pen.rx() += refGlyph[i].xadvance * scalefactor;
			}

			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * scalefactor ),
			                pen.y() + ( refGlyph[i].yoffset * scalefactor ) );

			if ( !m_RTL )
			{
				pen.rx() += refGlyph[i].xadvance * scalefactor;
			}
		}
	}

	delete otf;
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
// 	qDebug() << m_name  <<"::deRenderAll()";
// 	QSet<QGraphicsScene*> collectedScenes;
	for ( int i = 0; i < pixList.count(); ++i )
	{
		if ( pixList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			pixList[i]->scene()->removeItem ( pixList[i] );
			delete pixList[i];
		}
	}
	pixList.clear();
	for ( int i = 0; i < glyphList.count(); ++i )
	{
		if ( glyphList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			glyphList[i]->scene()->removeItem ( glyphList[i] );
			delete glyphList[i];
		}
	}
	glyphList.clear();
	for ( int i = 0; i < labList.count(); ++i )
	{
		if ( labList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			labList[i]->scene()->removeItem ( labList[i] );
			delete labList[i];
		}
	}
	labList.clear();
	for ( int i = 0; i < selList.count(); ++i )
	{
		if ( selList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			selList[i]->scene()->removeItem ( selList[i] );
			delete selList[i];
		}
	}
	selList.clear();
	allIsRendered = false;
// 	contourCache.clear();
	advanceCache.clear();
}

QByteArray FontItem::pixarray ( uchar * b, int len )
{
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

//Render all is dangerous ;)
// We now render langs

int FontItem::countCoverage ( int begin_code, int end_code )
{
	ensureFace();
	FT_ULong  charcode = begin_code ;
	FT_UInt   gindex = 0;
	int count = 0;
	if(begin_code >= 0)
	{
	if ( hasUnicode )
	{
		while ( charcode <= end_code )
		{
			charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
			if ( !gindex )
				break;
			++count;
		}
	}
	else
	{
		while ( charcode <= end_code )
		{
			if ( charcode < m_numGlyphs )
			{
				++charcode;
				++count;
			}
			else
				break;
		}
	}
	}
	else
	{
		FT_UInt anIndex = 1;
		QList<FT_UInt> notCovered;
		for(int i=1; i  < m_numGlyphs; ++i)
			notCovered << i;
		FT_UInt anyChar =  FT_Get_First_Char(m_face, &anIndex);
		while(anIndex)
		{ 
			anyChar =  FT_Get_Next_Char(m_face,anyChar,&anIndex);
			if(anIndex)
				notCovered.removeAll(anIndex);
		}	
		
		count = notCovered.count() ;
	}
	releaseFace();
	return count - 1;//something weird with freetype which put a valid glyph at the beginning of each lang ??? Or a bug here...
}

void FontItem::renderAll ( QGraphicsScene * scene , int begin_code, int end_code )
{
	ensureFace();
	if ( allIsRendered )
		return;
	deRenderAll();
	adjustGlyphsPerRow (scene->views()[0]->width() );
	QPointF pen ( 0,50 );
	int glyph_count = 0;
	int nl = 0;

// 	for ( int i=1;i<=m_numGlyphs; ++i )
// 		m_charLess.append ( i );

	FT_ULong  charcode;
	FT_UInt   gindex = 1;
	double sizz = 50;


// 	charcode = FT_Get_First_Char ( m_face, &gindex );
	charcode = begin_code;
// 	qDebug() << "INTER " << begin_code << end_code;
	QPen selPen ( Qt::gray );
	QFont infoFont ( "Helvetica",8 );
	QBrush selBrush ( QColor ( 255,255,255,0 ) );
	if(begin_code >= 0)
	{
	if ( hasUnicode )
	{
		while ( charcode <= end_code && gindex )
		{
			if ( nl == m_glyphsPerRow )
			{
				nl = 0;
				pen.rx() = 0;
				pen.ry() += 100;
			}
			QGraphicsPathItem *pitem = itemFromChar ( charcode , sizz );
			if ( pitem )
			{
				pitem->setData ( 1,"glyph" );
				pitem->setData ( 2,gindex );
				uint ucharcode = charcode;
				pitem->setData ( 3,ucharcode );
				glyphList.append ( pitem );
				scene->addItem ( pitem );
				pitem->setPos ( pen );
				pitem->setZValue ( 10 );

				QGraphicsTextItem *tit= scene->addText ( glyphName ( charcode )  + "\nU+" + QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) )  +" ("+ QString::number ( charcode ) +")"  , infoFont );
				tit->setPos ( pen.x()-10,pen.y() + 15 );
				tit->setData ( 1,"label" );
				tit->setData ( 2,gindex );
				tit->setData ( 3,ucharcode );
				labList.append ( tit );
				tit->setZValue ( 1 );

				QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
				rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
				rit->setData ( 1,"select" );
				rit->setData ( 2,gindex );
				rit->setData ( 3,ucharcode );
				rit->setZValue ( 100 );
				selList.append ( rit );

				pen.rx() += 100;
				++glyph_count;
				m_charLess.removeAll ( gindex );
				++nl;
			}
			charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
		}
	}
	else // Has not Unicode
	{
		// Here are fake charcodes (glyph index)
		while ( charcode <= end_code )
		{
			if ( nl == m_glyphsPerRow )
			{
				nl = 0;
				pen.rx() = 0;
				pen.ry() += 100;
			}
			QGraphicsPathItem *pitem = itemFromGindex ( charcode , sizz );
			if ( pitem )
			{
				pitem->setData ( 1,"glyph" );
				pitem->setData ( 2,gindex );
				pitem->setData ( 3,0 );
				glyphList.append ( pitem );
				scene->addItem ( pitem );
				pitem->setPos ( pen );
				pitem->setZValue ( 10 );

				QGraphicsTextItem *tit= scene->addText ( QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) ) , infoFont);
				tit->setPos ( pen.x(),pen.y() + 15 );
				tit->setData ( 1,"label" );
				tit->setData ( 2,gindex );
				tit->setData ( 3,0 );
				labList.append ( tit );
				tit->setZValue ( 1 );

				QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
				rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
				rit->setData ( 1,"select" );
				rit->setData ( 2,gindex );
				rit->setData ( 3,0 );
				rit->setZValue ( 100 );
				selList.append ( rit );

				pen.rx() += 100;
				++nl;
			}
			else
			{
				break;
			}
			++charcode;
		}
	}
	}
	else // beginCode is negative - it means search for out charmap glyphs
	{
		// 1/ where "out charmap" begins?
// 		int lastCharIndex = 0;
		FT_UInt anIndex = 1;
		QList<FT_UInt> notCovered;
		for(int i=1; i  < m_numGlyphs; ++i)
			notCovered << i;
		FT_UInt anyChar =  FT_Get_First_Char(m_face, &anIndex);
		while(anIndex)
		{ 
			anyChar =  FT_Get_Next_Char(m_face,anyChar,&anIndex);
			if(anIndex)
				notCovered.removeAll(anIndex);
		}	
		
		// 2/ fill with glyphs
		for(int i = 0; i < notCovered.count(); ++i)
		{
			if ( nl == m_glyphsPerRow )
			{
				nl = 0;
				pen.rx() = 0;
				pen.ry() += 100;
			}
			QGraphicsPathItem *pitem = itemFromGindex ( notCovered[i] , sizz );
			if ( pitem )
			{
				pitem->setData ( 1,"glyph" );
				pitem->setData ( 2, notCovered[i]);
				pitem->setData ( 3,0 );
				glyphList.append ( pitem );
				scene->addItem ( pitem );
				pitem->setPos ( pen );
				pitem->setZValue ( 10 );

				QGraphicsTextItem *tit= scene->addText ( QString ( "I+%1" ).arg ( notCovered[i] ), infoFont );
				tit->setPos ( pen.x(),pen.y() + 15 );
				tit->setData ( 1,"label" );
				tit->setData ( 2,notCovered[i]);
				tit->setData ( 3,0 );
				labList.append ( tit );
				tit->setZValue ( 1 );

				QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
				rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
				rit->setData ( 1,"select" );
				rit->setData ( 2,notCovered[i] );
				rit->setData ( 3,0 );
				rit->setZValue ( 100 );
				selList.append ( rit );

				pen.rx() += 100;
				++nl;
			}
			else
			{
				break;
			}
			++charcode;
		}
	}
	
	allIsRendered = true;
	releaseFace();
}

QString FontItem::infoText ( bool fromcache )
{
	if ( !m_cacheInfo.isEmpty() && fromcache )
		return m_cacheInfo;

	ensureFace();

	QMap<QString, QStringList> orderedInfo;
	QStringList tagsStr = m_tags;
	tagsStr.removeAll ( "Activated_On" );
	tagsStr.removeAll ( "Activated_Off" );
	QString ret ( "<h2 style=\"color:white;background-color:black;\">" + fancyName() + "</h2>\n" );
	ret += "<p>"+ QString::number ( m_numGlyphs ) + " glyphs || Type : "+ m_type +" || Charmaps : " + m_charsets.join ( ", " ) +"</p>";
	ret += "<p style=\"background-color:#aaa;\"><b>Tags  </b>"+ tagsStr.join ( " ; " ) +"</p>";

	if ( moreInfo.isEmpty() )
	{
		if ( testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" )
		{
			m_isOpenType = true;
			moreInfo_sfnt();
		}
		if ( m_path.endsWith ( ".pfb",Qt::CaseInsensitive ) )
		{
			moreInfo_type1();
		}
	}
// 	if ( !moreInfo.isEmpty() ) // moreInfo.isNotEmpty
	{
		QString sysLang = QLocale::languageToString ( QLocale::system ().language() ).toUpper();
		QString sysCountry = QLocale::countryToString ( QLocale::system ().country() ).toUpper();
		QString sysLoc = sysLang + "_"+ sysCountry;

		QString styleLangMatch;
		for ( QMap<int, QMap<QString, QString> >::const_iterator lit = moreInfo.begin(); lit != moreInfo.end(); ++lit )
		{
			if ( langIdMap[ lit.key() ].contains ( sysLang ) ) // lang match
			{
				styleLangMatch = " style=\"margin-left:16pt;\" ";
			}
			else if ( langIdMap[ lit.key() ] == "DEFAULT" ) // lang does not match but it’s international name
			{
				styleLangMatch = "style=\"margin-left:16pt;font-style:italic;\"";
			}
			else // lang does not match at all
			{
				styleLangMatch = "style=\"margin-left:16pt;font-style:italic;font-size:7pt\"";
			}
			for ( QMap<QString, QString>::const_iterator mit = lit.value().begin(); mit != lit.value().end(); ++mit )
			{
				if ( langIdMap[ lit.key() ].contains ( sysLang ) || langIdMap[ lit.key() ] == "DEFAULT" )
				{
					QString name_value = mit.value();
					name_value.replace ( "\n","<br/>" );
					orderedInfo[ mit.key() ] << "<p "+ styleLangMatch +">" + name_value +"</p>";
					if ( mit.key() == "Font Subfamily" )
						m_variant = mit.value();
				}
			}
		}
	}


	for ( int i = 1; i < name_meaning.count(); ++i )
	{
		if ( orderedInfo.contains ( name_meaning[i] ) )
		{
// 			qDebug() << orderedInfo[name_meaning[i]].join("|");
			QStringList entries ( orderedInfo[name_meaning[i]].toSet().toList() );
			ret += "<p>"/*+QString::number(i)+*/"<b>"+name_meaning[i] +"</b> "+entries.join ( " " ) +"</p>";
		}
		if ( i == 7 ) //trademark
			i = -1;
		if ( i == 0 ) //Copyright
			i = 7;
	}

	m_cacheInfo = ret;

	releaseFace();
	return ret;
}

QString FontItem::glyphName ( int codepoint )
{
	ensureFace();

	int index = FT_Get_Char_Index ( m_face, codepoint );
	if ( index== 0 )
	{
		return "noname";
	}

	QByteArray key ( 1001,0 );
	if ( FT_HAS_GLYPH_NAMES ( m_face ) )
	{
		FT_Get_Glyph_Name ( m_face, index, key.data() , 1000 );
		if ( key[0] == char ( 0 ) )
		{
			key = "noname";
		}
	}
	else
	{
		key = "noname";
	}
	return QString ( key );
	releaseFace();
}


QString FontItem::infoGlyph ( int index, int code )
{
	ensureFace();
	QString ret;
	ret += glyphName ( code ) ;
	ret += ", " + QObject::tr ( "codepoint is U+" ) ;
	ret += QString ( "%1" ).arg ( code, 4, 16, QChar ( 0x0030 ) ) ;
	ret += " (int"+ QString::number ( code ) +")";

	releaseFace();
	return ret;
}

//deprecated
QString FontItem::toElement()
{
	QString ret;
	ret = "<fontfile><file>%1</file><tag>%2</tag></fontfile>";
	return ret.arg ( name() ).arg ( tags().join ( "</tag><tag>" ) );
}

QGraphicsPathItem * FontItem::hasCodepoint ( int code )
{
	for ( int i=0;i< glyphList.count();++i )
	{
		if ( glyphList.at ( i )->data ( 3 ).toInt() == code )
			return glyphList.at ( i );
	}
	return 0;
}

QIcon  FontItem::oneLinePreviewIcon ( QString oneline )
{
	if ( !theOneLinePreviewIcon.isNull() )
		return theOneLinePreviewIcon;
	QRectF savedRect = theOneLineScene->sceneRect();
	theOneLineScene->setSceneRect ( 0,0,64,64 );

	renderLine ( theOneLineScene,oneline,QPointF ( 10,55 ),80,false );
	QPixmap apix ( 64,64 );
	apix.fill ( Qt::white );
	QPainter apainter ( &apix );
	apainter.setRenderHint ( QPainter::Antialiasing,true );
	theOneLineScene->render ( &apainter,apix.rect(),apix.rect() );
// 	theOneLinePreviewIcon.addPixmap(apix);
	theOneLinePreviewIcon = apix;

	theOneLineScene->removeItem ( theOneLineScene->createItemGroup ( theOneLineScene->items() ) );
	theOneLineScene->setSceneRect ( savedRect );
	return theOneLinePreviewIcon;
}

QPixmap FontItem::oneLinePreviewPixmap ( QString oneline )
{
	if ( !theOneLinePreviewPixmap.isNull() )
		return theOneLinePreviewPixmap;
	QRectF savedRect = theOneLineScene->sceneRect();
	theOneLineScene->setSceneRect ( 0,0,320,32 );

	if ( !m_rasterFreetype )
	{
		renderLine ( theOneLineScene,oneline ,QPointF ( 10,24 ),20,false );
		QPixmap apix ( 320,32 );
		apix.fill ( Qt::white );
		QPainter apainter ( &apix );
		apainter.setRenderHint ( QPainter::Antialiasing,true );
		theOneLineScene->render ( &apainter );
		theOneLinePreviewPixmap = apix;

		theOneLineScene->setSceneRect ( savedRect );
		theOneLineScene->removeItem ( theOneLineScene->createItemGroup ( theOneLineScene->items() ) );

	}
	else
	{
// 		qDebug()<< m_name << "renders " << oneline;
		ensureFace();
		FT_Set_Char_Size ( m_face, 20  * 64 , 0, 72, 72 );
		QPointF pen ( 10,0 );
		QPixmap linePixmap ( 320,32 );
		linePixmap.fill ( Qt::white );
		QPainter apainter ( &linePixmap );
		QVector<QRgb> palette;
		for ( int i =0;i < oneline.count() ; ++i )
		{
			int glyphIndex = FT_Get_Char_Index ( m_face, oneline[i].unicode() );
			if ( glyphIndex == 0 )
			{
				continue;
			}
			ft_error = FT_Load_Glyph ( m_face, glyphIndex, FT_LOAD_DEFAULT );
			if ( ft_error )
			{
				continue;
			}
			ft_error = FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL );
			if ( ft_error )
			{
				continue;
			}

			palette.clear();
			for ( int i = 0; i < m_face->glyph->bitmap.num_grays; ++i )
			{
				palette << qRgba ( 0,0,0, i );
			}
			QImage img ( m_face->glyph->bitmap.buffer,
			             m_face->glyph->bitmap.width,
			             m_face->glyph->bitmap.rows,
			             m_face->glyph->bitmap.pitch,
			             QImage::Format_Indexed8 );
			img.setColorTable ( palette );
// 			apainter.drawImage(pen.x(), pen.y() - m_face->glyph->bitmap_top, img);
			pen.ry() = 26 - m_face->glyph->bitmap_top;
			pen.rx() += m_face->glyph->bitmap_left;
			apainter.drawImage ( pen, img );
			pen.rx() +=  m_glyph->advance.x >> 6 ;
		}
		apainter.end();
		releaseFace();

		theOneLinePreviewPixmap = linePixmap;
	}

	if ( !theOneLinePreviewPixmap.isNull() )
		return theOneLinePreviewPixmap;

	theOneLinePreviewPixmap = QPixmap ( 320,32 );
	theOneLinePreviewPixmap.fill ( Qt::lightGray );
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

	if ( name_meaning.isEmpty() )
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
	int tname_count = FT_Get_Sfnt_Name_Count ( m_face );
	//TODO check encodings and platforms
	for ( int i=0; i < tname_count; ++i )
	{
		FT_Get_Sfnt_Name ( m_face,i,&tname );
		QString akey;
		if ( tname.name_id >  255 )
		{
// 			qDebug() << name() <<" has vendor’s specific name id ->" << tname.name_id;
			if ( tname.string_len > 0 )
			{
				akey = "VendorKey_" + QString::number ( tname.name_id );
			}
			else
			{
				continue;
			}

		}
		else if ( tname.name_id <= name_meaning.count() )
		{
			akey = name_meaning.at ( tname.name_id );
		}
		else
		{
// 			qDebug() << name() <<" : It seems there are new name IDs in TT spec ("<< tname.name_id <<")!";
			continue;
		}

		QString avalue;
		///New plan, we’ll put here _user contributed_ statements!
		if ( tname.platform_id ==TT_PLATFORM_MICROSOFT && tname.encoding_id == TT_MS_ID_UNICODE_CS ) // Corresponds to a Microsoft WGL4 charmap, matching Unicode.
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "UTF-16BE" );
			avalue = codec->toUnicode ( array );
		}
		else if ( tname.platform_id ==TT_PLATFORM_MICROSOFT && tname.encoding_id == TT_MS_ID_SYMBOL_CS ) // Corresponds to Microsoft symbol encoding. PM - don(t understand what it does here? seen in StandardSym.ttf
		{
			avalue = "Here, imagine some nice symbols!";
		}
		else if ( tname.platform_id == TT_PLATFORM_MACINTOSH  && tname.encoding_id == TT_APPLE_ID_DEFAULT ) // Unicode version 1.0
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "ISO 8859-15" ); // ### give better result than UTF ???
			avalue = codec->toUnicode ( array );
		}
		else if ( tname.platform_id == TT_PLATFORM_APPLE_UNICODE  && tname.encoding_id == TT_APPLE_ID_DEFAULT ) // Unicode version 1.0
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "ISO 8859-15" ); // ### give better result than UTF ???
			avalue = codec->toUnicode ( array );
		}
		else
		{
			avalue = "Unexpected platform - encoding pair ("
			         + QString::number ( tname.platform_id )
			         + "," + QString::number ( tname.encoding_id )
			         + ")\nPlease contact Fontmatrix team\nrun Fontmatrix in console to see more info";

			qDebug() << m_name
			<< "platform_id("
			<< tname.platform_id
			<<") - encoding_id("
			<< tname.encoding_id
			<<") - "
			<< QString::number ( tname.language_id )
			<< langIdMap[tname.language_id];
		}


		if ( !avalue.isEmpty() )
		{
			moreInfo[tname.language_id][akey] = avalue;
		}
	}
}

void FontItem::moreInfo_type1()
{
	PS_FontInfoRec sinfo ;
	int err = FT_Get_PS_Font_Info ( m_face,&sinfo );
	if ( err )
	{
		qDebug() <<"FT_Get_PS_Font_Info("<< m_name <<")"<<" failed :" << err;
		return;
	}
	// full_name version notice
	moreInfo[0]["Full font name"] = sinfo.full_name;
	moreInfo[0]["Version string"] = sinfo.version;
	moreInfo[0]["Description"] = sinfo.notice;
}

///return size of dynamic structuresttnameid.h
// int FontItem::debug_size()
// {
// // 	int ret=0;
// // 	for ( QMap<int,QPainterPath>::const_iterator cit = contourCache.begin(); cit != contourCache.end();++cit )
// // 		ret+=cit->elementCount();
//
// }

void FontItem::setTags ( QStringList l )
{
	m_tags = l;
	// overwrite cached info
	infoText ( false );
}

/// When glyphsView is resized we wantto adjust the number of columns
void FontItem::adjustGlyphsPerRow ( int width )
{
	m_glyphsPerRow = 1;
	int extraAdjust = 30;
	for ( int i = 1; i < 30 ; ++i )
	{
		if ( ( i*100 ) +extraAdjust > width )
			return;
		else
			m_glyphsPerRow = i;
	}
}

bool FontItem::isActivated()
{
	if ( m_tags.contains ( "Activated_Off" ) )
		return false;
	else if ( m_tags.contains ( "Activated_On" ) )
		return true;
	else
		m_tags << "Activated_Off";
	return false;
}

void FontItem::setActivated ( bool act )
{
	if ( act )
	{
		if ( isActivated() )
		{
			return;
		}
		else
		{
			m_tags.removeAll ( "Activated_Off" );
			m_tags << "Activated_On";
		}
	}
	else
	{
		if ( !isActivated() )
		{
			return;
		}
		else
		{
			m_tags.removeAll ( "Activated_On" );
			m_tags << "Activated_Off";
		}
	}
}


void FontItem::fillLangIdMap()
{
	if ( langIdMap.count() )
		return;
	langIdMap[0x0000] = "DEFAULT";
	langIdMap[0x0001] = "ARABIC_GENERAL                   ";
	langIdMap[0x0401] = "ARABIC_SAUDI_ARABIA              ";
	langIdMap[0x0801] = "ARABIC_IRAQ                      ";
	langIdMap[0x0c01] = "ARABIC_EGYPT                     ";
	langIdMap[0x1001] = "ARABIC_LIBYA                     ";
	langIdMap[0x1401] = "ARABIC_ALGERIA                   ";
	langIdMap[0x1801] = "ARABIC_MOROCCO                   ";
	langIdMap[0x1c01] = "ARABIC_TUNISIA                   ";
	langIdMap[0x2001] = "ARABIC_OMAN                      ";
	langIdMap[0x2401] = "ARABIC_YEMEN                     ";
	langIdMap[0x2801] = "ARABIC_SYRIA                     ";
	langIdMap[0x2c01] = "ARABIC_JORDAN                    ";
	langIdMap[0x3001] = "ARABIC_LEBANON                   ";
	langIdMap[0x3401] = "ARABIC_KUWAIT                    ";
	langIdMap[0x3801] = "ARABIC_UAE                       ";
	langIdMap[0x3c01] = "ARABIC_BAHRAIN                   ";
	langIdMap[0x4001] = "ARABIC_QATAR                     ";
	langIdMap[0x0402] = "BULGARIAN_BULGARIA               ";
	langIdMap[0x0403] = "CATALAN_SPAIN                    ";
	langIdMap[0x0004] = "CHINESE_GENERAL                  ";
	langIdMap[0x0404] = "CHINESE_TAIWAN                   ";
	langIdMap[0x0804] = "CHINESE_PRC                      ";
	langIdMap[0x0c04] = "CHINESE_HONG_KONG                ";
	langIdMap[0x1004] = "CHINESE_SINGAPORE                ";
	langIdMap[0x1404] = "CHINESE_MACAU                    ";
	langIdMap[TT_MS_LANGID_CHINESE_HONG_KONG] = "CHINESE_MACAU ";
	langIdMap[0x7C04] = "CHINESE_TRADITIONAL              ";
	langIdMap[0x0405] = "CZECH_CZECH_REPUBLIC             ";
	langIdMap[0x0406] = "DANISH_DENMARK                   ";
	langIdMap[0x0407] = "GERMAN_GERMANY                   ";
	langIdMap[0x0807] = "GERMAN_SWITZERLAND               ";
	langIdMap[0x0c07] = "GERMAN_AUSTRIA                   ";
	langIdMap[0x1007] = "GERMAN_LUXEMBOURG                ";
	langIdMap[0x1407] = "GERMAN_LIECHTENSTEI              ";
	langIdMap[0x0408] = "GREEK_GREECE                     ";
	langIdMap[0x2008] = "GREEK_GREECE2                    ";
	langIdMap[0x0009] = "ENGLISH_GENERAL                  ";
	langIdMap[0x0409] = "ENGLISH_UNITED_STATES            ";
	langIdMap[0x0809] = "ENGLISH_UNITED_KINGDOM           ";
	langIdMap[0x0c09] = "ENGLISH_AUSTRALIA                ";
	langIdMap[0x1009] = "ENGLISH_CANADA                   ";
	langIdMap[0x1409] = "ENGLISH_NEW_ZEALAND              ";
	langIdMap[0x1809] = "ENGLISH_IRELAND                  ";
	langIdMap[0x1c09] = "ENGLISH_SOUTH_AFRICA             ";
	langIdMap[0x2009] = "ENGLISH_JAMAICA                  ";
	langIdMap[0x2409] = "ENGLISH_CARIBBEAN                ";
	langIdMap[0x2809] = "ENGLISH_BELIZE                   ";
	langIdMap[0x2c09] = "ENGLISH_TRINIDAD                 ";
	langIdMap[0x3009] = "ENGLISH_ZIMBABWE                 ";
	langIdMap[0x3409] = "ENGLISH_PHILIPPINES              ";
	langIdMap[0x3809] = "ENGLISH_INDONESIA                ";
	langIdMap[0x3c09] = "ENGLISH_HONG_KONG                ";
	langIdMap[0x4009] = "ENGLISH_INDIA                    ";
	langIdMap[0x4409] = "ENGLISH_MALAYSIA                 ";
	langIdMap[0x4809] = "ENGLISH_SINGAPORE                ";
	langIdMap[0x040a] = "SPANISH_SPAIN_TRADITIONAL_SORT   ";
	langIdMap[0x080a] = "SPANISH_MEXICO                   ";
	langIdMap[0x0c0a] = "SPANISH_SPAIN_INTERNATIONAL_SORT ";
	langIdMap[0x100a] = "SPANISH_GUATEMALA                ";
	langIdMap[0x140a] = "SPANISH_COSTA_RICA               ";
	langIdMap[0x180a] = "SPANISH_PANAMA                   ";
	langIdMap[0x1c0a] = "SPANISH_DOMINICAN_REPUBLIC       ";
	langIdMap[0x200a] = "SPANISH_VENEZUELA                ";
	langIdMap[0x240a] = "SPANISH_COLOMBIA                 ";
	langIdMap[0x280a] = "SPANISH_PERU                     ";
	langIdMap[0x2c0a] = "SPANISH_ARGENTINA                ";
	langIdMap[0x300a] = "SPANISH_ECUADOR                  ";
	langIdMap[0x340a] = "SPANISH_CHILE                    ";
	langIdMap[0x380a] = "SPANISH_URUGUAY                  ";
	langIdMap[0x3c0a] = "SPANISH_PARAGUAY                 ";
	langIdMap[0x400a] = "SPANISH_BOLIVIA                  ";
	langIdMap[0x440a] = "SPANISH_EL_SALVADOR              ";
	langIdMap[0x480a] = "SPANISH_HONDURAS                 ";
	langIdMap[0x4c0a] = "SPANISH_NICARAGUA                ";
	langIdMap[0x500a] = "SPANISH_PUERTO_RICO              ";
	langIdMap[0x540a] = "SPANISH_UNITED_STATES            ";
	langIdMap[0xE40a] = "SPANISH_LATIN_AMERICA            ";
	langIdMap[0x040b] = "FINNISH_FINLAND                  ";
	langIdMap[0x040c] = "FRENCH_FRANCE                    ";
	langIdMap[0x080c] = "FRENCH_BELGIUM                   ";
	langIdMap[0x0c0c] = "FRENCH_CANADA                    ";
	langIdMap[0x100c] = "FRENCH_SWITZERLAND               ";
	langIdMap[0x140c] = "FRENCH_LUXEMBOURG                ";
	langIdMap[0x180c] = "FRENCH_MONACO                    ";
	langIdMap[0x1c0c] = "FRENCH_WEST_INDIES               ";
	langIdMap[0x200c] = "FRENCH_REUNION                   ";
	langIdMap[0x240c] = "FRENCH_CONGO                     ";
	langIdMap[TT_MS_LANGID_FRENCH_CONGO] = "FRENCH_ZAIRE ";
	langIdMap[0x280c] = "FRENCH_SENEGAL                   ";
	langIdMap[0x2c0c] = "FRENCH_CAMEROON                  ";
	langIdMap[0x300c] = "FRENCH_COTE_D_IVOIRE             ";
	langIdMap[0x340c] = "FRENCH_MALI                      ";
	langIdMap[0x380c] = "FRENCH_MOROCCO                   ";
	langIdMap[0x3c0c] = "FRENCH_HAITI                     ";
	langIdMap[0xE40c] = "FRENCH_NORTH_AFRICA              ";
	langIdMap[0x040d] = "HEBREW_ISRAEL                    ";
	langIdMap[0x040e] = "HUNGARIAN_HUNGARY                ";
	langIdMap[0x040f] = "ICELANDIC_ICELAND                ";
	langIdMap[0x0410] = "ITALIAN_ITALY                    ";
	langIdMap[0x0810] = "ITALIAN_SWITZERLAND              ";
	langIdMap[0x0411] = "JAPANESE_JAPAN                   ";
	langIdMap[0x0412] = "KOREAN_EXTENDED_WANSUNG_KOREA    ";
	langIdMap[0x0812] = "KOREAN_JOHAB_KOREA               ";
	langIdMap[0x0413] = "DUTCH_NETHERLANDS                ";
	langIdMap[0x0813] = "DUTCH_BELGIUM                    ";
	langIdMap[0x0414] = "NORWEGIAN_NORWAY_BOKMAL          ";
	langIdMap[0x0814] = "NORWEGIAN_NORWAY_NYNORSK         ";
	langIdMap[0x0415] = "POLISH_POLAND                    ";
	langIdMap[0x0416] = "PORTUGUESE_BRAZIL                ";
	langIdMap[0x0816] = "PORTUGUESE_PORTUGAL              ";
	langIdMap[0x0417] = "RHAETO_ROMANIC_SWITZERLAND       ";
	langIdMap[0x0418] = "ROMANIAN_ROMANIA                 ";
	langIdMap[0x0818] = "MOLDAVIAN_MOLDAVIA               ";
	langIdMap[0x0419] = "RUSSIAN_RUSSIA                   ";
	langIdMap[0x0819] = "RUSSIAN_MOLDAVIA                 ";
	langIdMap[0x041a] = "CROATIAN_CROATIA                 ";
	langIdMap[0x081a] = "SERBIAN_SERBIA_LATIN             ";
	langIdMap[0x0c1a] = "SERBIAN_SERBIA_CYRILLIC          ";
	langIdMap[0x101a] = "BOSNIAN_BOSNIA_HERZEGOVINA       ";
	langIdMap[0x101a] = "CROATIAN_BOSNIA_HERZEGOVINA      ";
	langIdMap[0x141a] = "BOSNIAN_BOSNIA_HERZEGOVINA       ";
	langIdMap[0x181a] = "SERBIAN_BOSNIA_HERZ_LATIN        ";
	langIdMap[0x181a] = "SERBIAN_BOSNIA_HERZ_CYRILLIC     ";
	langIdMap[0x041b] = "SLOVAK_SLOVAKIA                  ";
	langIdMap[0x041c] = "ALBANIAN_ALBANIA                 ";
	langIdMap[0x041d] = "SWEDISH_SWEDEN                   ";
	langIdMap[0x081d] = "SWEDISH_FINLAND                  ";
	langIdMap[0x041e] = "THAI_THAILAND                    ";
	langIdMap[0x041f] = "TURKISH_TURKEY                   ";
	langIdMap[0x0420] = "URDU_PAKISTAN                    ";
	langIdMap[0x0820] = "URDU_INDIA                       ";
	langIdMap[0x0421] = "INDONESIAN_INDONESIA             ";
	langIdMap[0x0422] = "UKRAINIAN_UKRAINE                ";
	langIdMap[0x0423] = "BELARUSIAN_BELARUS               ";
	langIdMap[0x0424] = "SLOVENE_SLOVENIA                 ";
	langIdMap[0x0425] = "ESTONIAN_ESTONIA                 ";
	langIdMap[0x0426] = "LATVIAN_LATVIA                   ";
	langIdMap[0x0427] = "LITHUANIAN_LITHUANIA             ";
	langIdMap[0x0827] = "CLASSIC_LITHUANIAN_LITHUANIA     ";
	langIdMap[0x0428] = "TAJIK_TAJIKISTAN                 ";
	langIdMap[0x0429] = "FARSI_IRAN                       ";
	langIdMap[0x042a] = "VIETNAMESE_VIET_NAM              ";
	langIdMap[0x042b] = "ARMENIAN_ARMENIA                 ";
	langIdMap[0x042c] = "AZERI_AZERBAIJAN_LATIN           ";
	langIdMap[0x082c] = "AZERI_AZERBAIJAN_CYRILLIC        ";
	langIdMap[0x042d] = "BASQUE_SPAIN                     ";
	langIdMap[0x042e] = "SORBIAN_GERMANY                  ";
	langIdMap[0x042f] = "MACEDONIAN_MACEDONIA             ";
	langIdMap[0x0430] = "SUTU_SOUTH_AFRICA                ";
	langIdMap[0x0431] = "TSONGA_SOUTH_AFRICA              ";
	langIdMap[0x0432] = "TSWANA_SOUTH_AFRICA              ";
	langIdMap[0x0433] = "VENDA_SOUTH_AFRICA               ";
	langIdMap[0x0434] = "XHOSA_SOUTH_AFRICA               ";
	langIdMap[0x0435] = "ZULU_SOUTH_AFRICA                ";
	langIdMap[0x0436] = "AFRIKAANS_SOUTH_AFRICA           ";
	langIdMap[0x0437] = "GEORGIAN_GEORGIA                 ";
	langIdMap[0x0438] = "FAEROESE_FAEROE_ISLANDS          ";
	langIdMap[0x0439] = "HINDI_INDIA                      ";
	langIdMap[0x043a] = "MALTESE_MALTA                    ";
	langIdMap[0x043b] = "SAMI_NORTHERN_NORWAY             ";
	langIdMap[0x083b] = "SAMI_NORTHERN_SWEDEN             ";
	langIdMap[0x0C3b] = "SAMI_NORTHERN_FINLAND            ";
	langIdMap[0x103b] = "SAMI_LULE_NORWAY                 ";
	langIdMap[0x143b] = "SAMI_LULE_SWEDEN                 ";
	langIdMap[0x183b] = "SAMI_SOUTHERN_NORWAY             ";
	langIdMap[0x1C3b] = "SAMI_SOUTHERN_SWEDEN             ";
	langIdMap[0x203b] = "SAMI_SKOLT_FINLAND               ";
	langIdMap[0x243b] = "SAMI_INARI_FINLAND               ";
	langIdMap[0x043b] = "SAAMI_LAPONIA                    ";
	langIdMap[0x043c] = "IRISH_GAELIC_IRELAND             ";
	langIdMap[0x083c] = "SCOTTISH_GAELIC_UNITED_KINGDOM   ";
	langIdMap[0x083c] = "SCOTTISH_GAELIC_UNITED_KINGDOM   ";
	langIdMap[0x043c] = "IRISH_GAELIC_IRELAND             ";
	langIdMap[0x043d] = "YIDDISH_GERMANY                  ";
	langIdMap[0x043e] = "MALAY_MALAYSIA                   ";
	langIdMap[0x083e] = "MALAY_BRUNEI_DARUSSALAM          ";
	langIdMap[0x043f] = "KAZAK_KAZAKSTAN                  ";
	langIdMap[0x0440] = "KIRGHIZ_KIRGHIZSTAN";
	langIdMap[0x0441] = "SWAHILI_KENYA                    ";
	langIdMap[0x0442] = "TURKMEN_TURKMENISTAN             ";
	langIdMap[0x0443] = "UZBEK_UZBEKISTAN_LATIN           ";
	langIdMap[0x0843] = "UZBEK_UZBEKISTAN_CYRILLIC        ";
	langIdMap[0x0444] = "TATAR_TATARSTAN                  ";
	langIdMap[0x0445] = "BENGALI_INDIA                    ";
	langIdMap[0x0845] = "BENGALI_BANGLADESH               ";
	langIdMap[0x0446] = "PUNJABI_INDIA                    ";
	langIdMap[0x0846] = "PUNJABI_ARABIC_PAKISTAN          ";
	langIdMap[0x0447] = "GUJARATI_INDIA                   ";
	langIdMap[0x0448] = "ORIYA_INDIA                      ";
	langIdMap[0x0449] = "TAMIL_INDIA                      ";
	langIdMap[0x044a] = "TELUGU_INDIA                     ";
	langIdMap[0x044b] = "KANNADA_INDIA                    ";
	langIdMap[0x044c] = "MALAYALAM_INDIA                  ";
	langIdMap[0x044d] = "ASSAMESE_INDIA                   ";
	langIdMap[0x044e] = "MARATHI_INDIA                    ";
	langIdMap[0x044f] = "SANSKRIT_INDIA                   ";
	langIdMap[0x0450] = "MONGOLIAN_MONGOLIA /* Cyrillic */";
	langIdMap[0x0850] = "MONGOLIAN_MONGOLIA_MONGOLIAN     ";
	langIdMap[0x0451] = "TIBETAN_CHINA                    ";
	/* TT_MS_LANGID_TIBETAN_BHUTAN is correct, BTW.    */
	langIdMap[0x0851] = "DZONGHKA_BHUTAN                  ";
	langIdMap[0x0451] = "TIBETAN_BHUTAN                   ";
	langIdMap[TT_MS_LANGID_DZONGHKA_BHUTAN] = "TIBETAN_BHUTAN  ";
	langIdMap[0x0452] = "WELSH_WALES                      ";
	langIdMap[0x0453] = "KHMER_CAMBODIA                   ";
	langIdMap[0x0454] = "LAO_LAOS                         ";
	langIdMap[0x0455] = "BURMESE_MYANMAR                  ";
	langIdMap[0x0456] = "GALICIAN_SPAIN                   ";
	langIdMap[0x0457] = "KONKANI_INDIA                    ";
	langIdMap[0x0458] = "MANIPURI_INDIA  /* Bengali */    ";
	langIdMap[0x0459] = "SINDHI_INDIA /* Arabic */        ";
	langIdMap[0x0859] = "SINDHI_PAKISTAN                  ";
	langIdMap[0x045a] = "SYRIAC_SYRIA                     ";
	langIdMap[0x045b] = "SINHALESE_SRI_LANKA              ";
	langIdMap[0x045c] = "CHEROKEE_UNITED_STATES           ";
	langIdMap[0x045d] = "INUKTITUT_CANADA                 ";
	langIdMap[0x045e] = "AMHARIC_ETHIOPIA                 ";
	langIdMap[0x045f] = "TAMAZIGHT_MOROCCO /* Arabic */   ";
	langIdMap[0x085f] = "TAMAZIGHT_MOROCCO_LATIN          ";
	langIdMap[0x0460] = "KASHMIRI_PAKISTAN /* Arabic */   ";
	langIdMap[0x0860] = "KASHMIRI_SASIA                   ";
	langIdMap[TT_MS_LANGID_KASHMIRI_SASIA] = "KASHMIRI_INDIA";
	langIdMap[0x0461] = "NEPALI_NEPAL                     ";
	langIdMap[0x0861] = "NEPALI_INDIA                     ";
	langIdMap[0x0462] = "FRISIAN_NETHERLANDS              ";
	langIdMap[0x0463] = "PASHTO_AFGHANISTAN               ";
	langIdMap[0x0464] = "FILIPINO_PHILIPPINES             ";
	langIdMap[0x0465] = "DHIVEHI_MALDIVES                 ";
	langIdMap[TT_MS_LANGID_DHIVEHI_MALDIVES] = "DIVEHI_MALDIVES ";
	langIdMap[0x0466] = "EDO_NIGERIA                      ";
	langIdMap[0x0467] = "FULFULDE_NIGERIA                 ";
	langIdMap[0x0468] = "HAUSA_NIGERIA                    ";
	langIdMap[0x0469] = "IBIBIO_NIGERIA                   ";
	langIdMap[0x046a] = "YORUBA_NIGERIA                   ";
	langIdMap[0x046b] = "QUECHUA_BOLIVIA                  ";
	langIdMap[0x086b] = "QUECHUA_ECUADOR                  ";
	langIdMap[0x0c6b] = "QUECHUA_PERU                     ";
	langIdMap[0x046c] = "SEPEDI_SOUTH_AFRICA              ";
	langIdMap[0x0470] = "IGBO_NIGERIA                     ";
	langIdMap[0x0471] = "KANURI_NIGERIA                   ";
	langIdMap[0x0472] = "OROMO_ETHIOPIA                   ";
	langIdMap[0x0473] = "TIGRIGNA_ETHIOPIA                ";
	langIdMap[0x0873] = "TIGRIGNA_ERYTHREA                ";
	langIdMap[TT_MS_LANGID_TIGRIGNA_ERYTHREA] = "TIGRIGNA_ERYTREA ";
	langIdMap[0x0474] = "GUARANI_PARAGUAY                 ";
	langIdMap[0x0475] = "HAWAIIAN_UNITED_STATES           ";
	langIdMap[0x0476] = "LATIN                            ";
	langIdMap[0x0477] = "SOMALI_SOMALIA                   ";
	langIdMap[0x0478] = "YI_CHINA                         ";
	langIdMap[0x0479] = "PAPIAMENTU_NETHERLANDS_ANTILLES  ";
	langIdMap[0x0480] = "UIGHUR_CHINA                     ";
	langIdMap[0x0481] = "MAORI_NEW_ZEALAND                ";
	langIdMap[0x04ff] = "HUMAN_INTERFACE_DEVICE           ";

}

FmOtf * FontItem::takeOTFInstance()
{
	ensureFace();
	if ( m_isOpenType )
		otf = new FmOtf ( m_face );
	return otf;

	// It is a case where we don’t release face, thr caller have to call releaseOTFInstance;
}

void FontItem::releaseOTFInstance ( FmOtf * rotf )
{
	if ( rotf == otf )
		delete otf;
	releaseFace();
}














