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
#include "fmaltcontext.h"
#include "fmotf.h"
#include "fmencdata.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "fmfreetypelib.h"
#include "fmglyphsview.h"
#include "glyphtosvghelper.h"
#include "typotek.h"
#include "fmbaseshaper.h"
#include "hyphenate/fmhyphenator.h"
#include "fmkernfeat.h"
#include "fmuniblocks.h"

#include <cmath>

#include <QDebug>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QApplication>
#include <QPainter>
#include <QLocale>
#include <QTextCodec>

#include <QProgressDialog>
#include <QHttp>

#include "QDebug"

#include FT_XFREE86_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TYPE1_TABLES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H

// #include <QWaitCondition>
// #include <QMutex>

int fm_num_face_opened = 0;

QGraphicsScene *FontItem::theOneLineScene = 0;





QList<int> FontItem::legitimateNonPathChars;

QVector<QRgb> gray256Palette;
QVector<QRgb> invertedGray256Palette;

// QWaitCondition theCondition;
// QMutex theMutex;

unsigned int OTF_name_tag ( QString s );

/** functions set for decomposition
 */

struct SizedPath{
	QPainterPath* p;
	double s;
};

// an anticipation of further changes in Freetype
struct FM_Vector // :)
{
	double x;
	double y;
	
	FM_Vector(const FT_Vector* vect)
	{
		x = double(vect->x); 
		y = double(vect->y); 
		
// 		qDebug()<<"x26"<<vect26dot6->x<<"y26"<<vect26dot6->y <<"x"<<x<<"y"<<y;
	};
};

static int _moveTo ( const FT_Vector*  to26, void*   user )
{
	FM_Vector to(to26);
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p( sp->p );
	double sf( sp->s );
	p->moveTo ( to.x * sf , to.y * sf * -1.0 );
	return 0;
}
static int _lineTo ( const FT_Vector*  to26, void*   user )
{
	FM_Vector to(to26);
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p( sp->p );
	double sf( sp->s );
	p->lineTo ( to.x * sf, to.y  * sf * -1.0 );
	return  0;
}
static int _conicTo ( const FT_Vector* control26, const FT_Vector*  to26, void*   user )
{
	FM_Vector control(control26);
	FM_Vector to(to26);
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p( sp->p );
	double sf( sp->s );
	p->quadTo ( control.x * sf,control.y * sf * -1.0,to.x * sf,to.y * sf * -1.0 );
	return 0;
}
static int _cubicTo ( const FT_Vector* control126, const FT_Vector* control226, const FT_Vector*  to26, void*   user )
{
	FM_Vector control1(control126);
	FM_Vector control2(control226);
	FM_Vector to(to26);
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p( sp->p );
	double sf( sp->s );
	p->cubicTo ( control1.x * sf,control1.y * sf * -1.0,control2.x * sf,control2.y * sf * -1.0,to.x * sf,to.y  * sf * -1.0);
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



void FontItem::fillLegitimateSpaces()
{
	legitimateNonPathChars << 0x0020 ;
	legitimateNonPathChars << 0x00A0 ;
	legitimateNonPathChars << 0x1680 ;
	legitimateNonPathChars << 0x180E ;
	legitimateNonPathChars << 0x2002 ;
	legitimateNonPathChars << 0x2003 ;
	legitimateNonPathChars << 0x2004 ;
	legitimateNonPathChars << 0x2005 ;
	legitimateNonPathChars << 0x2006 ;
	legitimateNonPathChars << 0x2007 ;
	legitimateNonPathChars << 0x2008 ;
	legitimateNonPathChars << 0x2009 ;
	legitimateNonPathChars << 0x200A ;
	legitimateNonPathChars << 0x200B ;
	legitimateNonPathChars << 0x200C ;
	legitimateNonPathChars << 0x200D ;
	legitimateNonPathChars << 0x202F ;
	legitimateNonPathChars << 0x205F ;
	legitimateNonPathChars << 0x2060 ;
	legitimateNonPathChars << 0x3000 ;
	legitimateNonPathChars << 0xFEFF ;
}


void FontItem::fill256Palette()
{
// #ifdef   PLATFORM_APPLE
// 	for ( int i = 0; i < 256 ; ++i )
// 	{
// 		gray256Palette << qRgb (255-i, 255-i,255- i );
// 	}
// #else
	for ( int i = 0; i < 256 ; ++i )
	{
		gray256Palette << qRgba ( 0,0,0, i );
	}
// #endif
}

void FontItem::fillInvertedPalette()
{
	for ( int i = 0; i < 256 ; ++i )
	{
		invertedGray256Palette << qRgb ( i , i,  i );
	}
}




FontItem::FontItem ( QString path , bool remote, bool faststart )
{
// 	qDebug()<<"FONT ITEM"<<path;
	m_valid = false;
	m_active = false;
	m_remote = remote;
	remoteCached = false;
	stopperDownload = false;
	m_face = 0;
	lastFace = 0;
	m_glyphsPerRow = 5;
	m_isEncoded = false;
	currentChar = -1;
	m_isOpenType = false;
	otf = 0;
	m_rasterFreetype = false;
	m_progression = PROGRESSION_LTR;
	m_shaperType = 1;
	renderReturnWidth = false;
	unitPerEm = 0;
	m_FTHintMode = 0;
	allIsRendered = false;
	isUpToDate = false;
	m_path = path;

	/// STATIC INITIALISATIONS

	if ( legitimateNonPathChars.isEmpty() )
		fillLegitimateSpaces();
	if ( gray256Palette.isEmpty() )
		fill256Palette();
	if ( invertedGray256Palette.isEmpty() )
		fillInvertedPalette();
	if ( !theOneLineScene )
	{
		theOneLineScene = new QGraphicsScene;
	}
	/// EndOF S I

	if ( m_remote || faststart )
	{
		m_valid = true;
		return;
	}
	

	QFileInfo infopath ( m_path );
	m_name = infopath.fileName();
	m_fileSize = QString::number( infopath.size() , 10 ) ;

	if ( ! ensureFace() )
	{
		return;
	}


	if ( infopath.suffix() == "pfb" || infopath.suffix() == "PFB" )
	{
		m_afm = m_path;
		if ( infopath.suffix() == "pfb" )
		{
			m_afm.replace ( ".pfb",".afm" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".afm" ,".AFM" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
		else if ( infopath.suffix() == "PFB" )
		{
			m_afm.replace ( ".PFB",".AFM" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".AFM" ,".afm" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
	}


	if ( testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" )
	{
		m_isOpenType = true;
	}
	
	if(m_isOpenType)
		moreInfo_sfnt();
	else
		moreInfo_type1();

	m_type = FT_Get_X11_Font_Format ( m_face ); 
// 	if ( typotek::getInstance()->familySchemeFreetype() || !m_isOpenType )
	{
		m_family = m_face->family_name;
		m_variant = m_face->style_name;
	}
// 	else
// 	{
// 		m_family = getAlternateFamilyName();
// 		m_variant = getAlternateVariantName();
// 	}
	m_numGlyphs = m_face->num_glyphs;
	m_numFaces = m_face->num_faces;

// 	for ( int i = 0 ;i < m_face->num_charmaps; ++i )
// 	{
// 		m_charsets << charsetMap[m_face->charmaps[i]->encoding];
// 	}


// 	m_lock = false;
	pixList.clear();

	if ( m_family.isEmpty() )
		return;
	if ( m_variant.isEmpty() )
		return;

	m_valid = true;
	releaseFace();
}

FontItem::FontItem(QString path, QString family, QString variant, QString type,bool active)
{
	m_valid = true;
	m_remote = false;
	remoteCached = false;
	stopperDownload = false;
	m_face = 0;
	lastFace = 0;
	m_glyphsPerRow = 5;
	m_isEncoded = false;
	currentChar = -1;
	m_isOpenType = false;
	otf = 0;
	m_rasterFreetype = false;
	m_progression = PROGRESSION_LTR;
	m_shaperType = 1;
	renderReturnWidth = false;
	unitPerEm = 0;
	m_FTHintMode = 0;
// 	m_lock = false;
	allIsRendered = false;
	isUpToDate = false;
	

	if ( legitimateNonPathChars.isEmpty() )
		fillLegitimateSpaces();
	if ( gray256Palette.isEmpty() )
		fill256Palette();
	if ( invertedGray256Palette.isEmpty() )
		fillInvertedPalette();
	if ( !theOneLineScene )
	{
		theOneLineScene = new QGraphicsScene;
	}
	
	m_path = path;	
	m_family = family;
	m_variant = variant;
	m_active = active;
	m_type = type;
}

FontItem * FontItem::Clone()
{
	FontItem *fitem = new FontItem ( m_path, m_family, m_variant, m_type, m_active );
	return fitem;
}

void FontItem::updateItem()
{
	if(isUpToDate)
		return;
	QFileInfo infopath ( m_path );
	m_name = infopath.fileName();
	if ( ! ensureFace() )
	{
		return;
	}
	if ( infopath.suffix() == "pfb" || infopath.suffix() == "PFB" )
	{
		m_afm = m_path;
		if ( infopath.suffix() == "pfb" )
		{
			m_afm.replace ( ".pfb",".afm" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".afm" ,".AFM" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
		else if ( infopath.suffix() == "PFB" )
		{
			m_afm.replace ( ".PFB",".AFM" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".AFM" ,".afm" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
	}


	if ( testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" )
	{
		m_isOpenType = true;
	}

	m_type = FT_Get_X11_Font_Format ( m_face );
	m_family = m_face->family_name;
	m_variant = m_face->style_name;
	m_numGlyphs = m_face->num_glyphs;
	m_numFaces = m_face->num_faces;
	
	releaseFace();
	isUpToDate = true;
}

FontItem::~FontItem()
{
	if ( m_isOpenType && otf )
	{
// 		delete otf;
	}
}


void FontItem::encodeFace()
{
	if(!m_face)
		return;
	
	m_charsets.clear();
	m_unicodeBuiltIn = (m_face->charmap == NULL) ? false : true ;
	if(QString(FT_Get_X11_Font_Format(m_face)) == QString("Type 1"))
		m_unicodeBuiltIn = false;
	
	QMap<FT_Encoding, FT_CharMap> cmaps;
	for(int u = 0; u < m_face->num_charmaps; u++)
	{
		cmaps [ m_face->charmaps[u]->encoding ] = m_face->charmaps[u];
	}
	
	bool mapped(false);
	// Stop kidding
	if (/* (!isType1) && UnicodeBuiltIn && */cmaps.contains( FT_ENCODING_UNICODE ) )
	{
		FT_Set_Charmap(m_face, cmaps[FT_ENCODING_UNICODE]);
		m_charsets << FT_ENCODING_UNICODE;
		mapped = true;
		m_isEncoded = true;
		m_currentEncoding = FT_ENCODING_UNICODE;
	}
	// uncomment below to get Unicode cmap synthetized by FT
// 	else if(cmaps.contains( FT_ENCODING_UNICODE ))
// 	{
// 		FT_Set_Charmap(m_face, cmaps[FT_ENCODING_UNICODE]);
// 		m_charsets << FontStrings::Encoding(FT_ENCODING_UNICODE) +"*";
// 		mapped = true;
// 		cmaps.remove(FT_ENCODING_UNICODE);
// 		m_isEncoded = true;
// 	}
	foreach(FT_Encoding e, cmaps.keys())
	{
// 		QString cs(FontStrings::Encoding(e));
// 		if(isType1 && (e == FT_ENCODING_UNICODE))
// 			continue;
		if(!m_charsets.contains(e))
			m_charsets << e;
		if(!mapped)
		{
			FT_Set_Charmap(m_face, cmaps[e]);
			mapped = true;
			m_isEncoded = true;
			m_currentEncoding = e;
		}
	}
}

bool FontItem::ensureFace()
{
	FT_Library ftlib = FMFreetypeLib::lib(thread());
//	qDebug()<<"FontItem::ensureFace"<<thread();

	if ( m_face )
	{
		++facesRef;
		return true;
	}
	QString trueFile ( m_remote ? remoteHerePath : m_path );
	ft_error = FT_New_Face ( ftlib, trueFile.toLocal8Bit() , 0, &m_face );
	if ( ft_error )
	{
		qDebug() << "Error loading face [" << trueFile <<"]";
		return false;
	}
	encodeFace();
	if ( spaceIndex.isEmpty() )
	{
		int gIndex ( 0 );
		for ( int i ( 0 ); i < legitimateNonPathChars.count(); ++i )
		{
			gIndex =   FT_Get_Char_Index ( m_face , legitimateNonPathChars[i] );
			if ( gIndex )
			{
				spaceIndex << gIndex;
			}
		}
	}
	unitPerEm = m_face->units_per_EM;
	m_glyph = m_face->glyph;
	facesRef = 1;
	++fm_num_face_opened;
	return true;
}

void FontItem::releaseFace()
{
	if ( m_face )
	{
		--facesRef;
		if ( facesRef == 0 )
		{
			FT_Done_Face ( m_face );
			m_face = 0;
			--fm_num_face_opened;
		}
	}
}

int FontItem::glyphsCount() const
{
	if(m_numGlyphs > 0) // this is normal case
	{
		return m_numGlyphs;
	}
	FontItem * that(const_cast<FontItem*>(this));
	that->ensureFace();
	that->m_numGlyphs = m_face->num_glyphs;
	that->releaseFace();
	return m_numGlyphs;
}

QString FontItem::testFlag ( long flag, long against, QString yes, QString no )
{
	if ( ( flag & against ) == against )
		return yes;
	else
		return no;
}

// QString FontItem::value ( QString k )
// {
// 	// I don’t know if something relies o it so I keep it, for the moment.
// 	if ( k == "family" )
// 		return m_family;
// 	else if ( k == "variant" )
// 		return m_variant;
// 
// 	// 0 is default language
// 	// TODO inspect all available languages
// // 	if(moreInfo.isEmpty())
// // 	{
// // 		if(isOpenType())
// // 			moreInfo_sfnt();
// // 		else
// // 			moreInfo_type1();
// // 	}
// 	FontInfoMap moreInfo( FMFontDb::DB()->getInfoMap(m_path) );
// 	QMap<int, QString> namap ( moreInfo.value ( 0 ) );
// 	return namap.value ( name_meaning.indexOf( k ) );
// }

// QString FontItem::panose ( QString k )
// {
// 	return panoseInfo.value ( k );
// }


QString FontItem::name()
{
	return m_name;
}

QGraphicsPathItem * FontItem::itemFromChar ( int charcode, double size )
{

	if(!ensureFace())
		return 0;
	uint glyphIndex = 0;
	currentChar = charcode;
	glyphIndex = FT_Get_Char_Index ( m_face, charcode );

	QGraphicsPathItem * ret(itemFromGindex ( glyphIndex,size ));
	releaseFace();
	return ret;

}

QGraphicsPathItem * FontItem::itemFromGindex ( int index, double size )
{
	if(!ensureFace())
		return 0;
	int charcode = index ;
	double scalefactor = size / m_face->units_per_EM;
	ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_NO_SCALE );
	if ( ft_error )
	{
		QPainterPath glyphPath;
		glyphPath.addRect ( 0.0,0.0, size, size );
		QGraphicsPathItem *glyph = new  QGraphicsPathItem;
		glyph->setBrush ( QBrush ( Qt::red ) );
		glyph->setPath ( glyphPath );
		glyph->setData ( GLYPH_DATA_GLYPH, index);
		glyph->setData ( GLYPH_DATA_HADVANCE , ( double ) size * scalefactor);
		glyph->setData ( GLYPH_DATA_HADVANCE_SCALED , ( double ) size );
		glyph->setData ( GLYPH_DATA_ERROR , true );
		releaseFace();
		return glyph;
	}

	FT_Outline *outline = &m_glyph->outline;
	QPainterPath glyphPath ( QPointF ( 0.0,0.0 ) );
	SizedPath sp;
	sp.p = &glyphPath;
	sp.s = scalefactor;
	FT_Outline_Decompose ( outline, &outline_funcs, &sp );
	glyphPath.closeSubpath();
	QGraphicsPathItem *glyph = new  QGraphicsPathItem;

	if ( glyphPath.elementCount() < 3 && !spaceIndex.contains ( index ) )
	{
		QBrush brush ( Qt::SolidPattern );
		brush.setColor ( Qt::red );
		QPen pen ( brush, 0 );
		QPainterPath errPath;
		errPath.addRect ( 0.0,-size, size , size );
		glyph->setBrush ( brush );
		glyph->setPen ( pen );
		glyph->setPath ( errPath );
		glyph->setData ( GLYPH_DATA_HADVANCE , ( double ) m_glyph->metrics.horiAdvance   );
		glyph->setData ( GLYPH_DATA_HADVANCE_SCALED , ( double ) m_glyph->metrics.horiAdvance  *scalefactor);
		glyph->setData ( GLYPH_DATA_GLYPH, index);
		glyph->setData ( GLYPH_DATA_ERROR , true );
	}
	else
	{
		glyph->setBrush ( QBrush ( Qt::SolidPattern ) );
		glyph->setPath ( glyphPath );
		glyph->setData ( GLYPH_DATA_GLYPH, index);
		glyph->setData ( GLYPH_DATA_HADVANCE , ( double ) m_glyph->metrics.horiAdvance );
		glyph->setData ( GLYPH_DATA_HADVANCE_SCALED , ( double ) m_glyph->metrics.horiAdvance * scalefactor);
		glyph->setData ( 5, ( double ) m_glyph->metrics.vertAdvance );
		glyph->setData ( GLYPH_DATA_ERROR , false );
// 		glyph->scale ( scalefactor,-scalefactor );
	}
	releaseFace();
	return glyph;
}

QGraphicsPixmapItem * FontItem::itemFromCharPix ( int charcode, double size )
{
	if(!ensureFace())
		return 0;
	uint glyphIndex = 0;
	currentChar = charcode;
	glyphIndex = FT_Get_Char_Index ( m_face, charcode );

	releaseFace();
	return itemFromGindexPix ( glyphIndex,size );

}


QGraphicsPixmapItem * FontItem::itemFromGindexPix ( int index, double size )
{
	if ( !ensureFace() )
		return 0;
	int charcode = index ;

	double scaleFactor = size / m_face->units_per_EM;

	// Set size
	FT_Set_Char_Size ( m_face,
	                   qRound( size  * 64 ),
	                   0,
			   typotek::getInstance()->getDpiX(),
			   typotek::getInstance()->getDpiY() );

	// Grab metrics in FONT UNIT
	ft_error = FT_Load_Glyph ( m_face,
	                           charcode  ,
	                           FT_LOAD_NO_SCALE  );
	if ( ft_error )
	{
		QPixmap square ( qRound(size) , qRound(size) );
		square.fill ( Qt::red );
		QGraphicsPixmapItem *glyph = new QGraphicsPixmapItem ( square );
		glyph->setData ( GLYPH_DATA_GLYPH ,index );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setData ( GLYPH_DATA_HADVANCE ,size / ( size / m_face->units_per_EM ) );
		releaseFace();
		return glyph;
	}

	double takeAdvanceBeforeRender = m_glyph->metrics.horiAdvance * ( typotek::getInstance()->getDpiX() / 72.0 );
	double takeVertAdvanceBeforeRender = m_glyph->metrics.vertAdvance * ( typotek::getInstance()->getDpiY() / 72.0 );
	double takeLeftBeforeRender = double(m_glyph->metrics.horiBearingX) * ( typotek::getInstance()->getDpiX() / 72.0 );
	
// 	if(m_FTHintMode != FT_LOAD_NO_HINTING)
	{
		ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_DEFAULT | m_FTHintMode  );
	}
	// Render the glyph into a grayscale bitmap
	ft_error = FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL );
	if ( ft_error )
	{
		QPixmap square ( qRound(size) , qRound(size) );
		square.fill ( Qt::red );
		QGraphicsPixmapItem *glyph = new QGraphicsPixmapItem ( square );
		glyph->setData ( GLYPH_DATA_GLYPH , index );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setData ( GLYPH_DATA_HADVANCE ,size  / ( size / m_face->units_per_EM ) );
		releaseFace();
		return glyph;
	}


	QImage img ( glyphImage() );
	QGraphicsPixmapItem *glyph = new  QGraphicsPixmapItem;

	if ( img.isNull() && !spaceIndex.contains ( index ) )
	{
		QPixmap square ( qRound(size) , qRound(size) );
		square.fill ( Qt::red );
		glyph->setPixmap ( square );
		glyph->setData ( GLYPH_DATA_GLYPH , index );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setData ( GLYPH_DATA_HADVANCE ,size / ( size / m_face->units_per_EM ) );
	}
	else
	{
#ifndef PLATFORM_APPLE
		glyph->setPixmap ( QPixmap::fromImage ( img ) );
#else
		QPixmap aPix ( img.width(), img.height() );
		aPix.fill ( QColor ( 0,0,0,0 ) );
		QPainter aPainter ( &aPix );
		aPainter.drawImage ( 0,0, img );
		glyph->setPixmap ( aPix );
#endif
		// we need to transport more data
		glyph->setData ( GLYPH_DATA_GLYPH , index );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , takeLeftBeforeRender );
		glyph->setData ( GLYPH_DATA_BITMAPTOP , double(m_face->glyph->bitmap_top) );
		glyph->setData ( GLYPH_DATA_HADVANCE , takeAdvanceBeforeRender );
		glyph->setData ( GLYPH_DATA_VADVANCE , takeVertAdvanceBeforeRender );
	}

	releaseFace();
	return glyph;
}

MetaGlyphItem * FontItem::itemFromGindexPix_mt(int index, double size)
{
	if ( !ensureFace() )
		return 0;
	int charcode = index ;
//	qDebug()<<"FontItem::itemFromGindexPix_mt"<< thread();
	MetaGlyphItem * glyph = new MetaGlyphItem;
	double scaleFactor = size / m_face->units_per_EM;

	// Set size
	FT_Set_Char_Size ( m_face,
			   qRound( size  * 64 ),
			   0,
			   typotek::getInstance()->getDpiX(),
			   typotek::getInstance()->getDpiY() );

	// Grab metrics in FONT UNIT
	ft_error = FT_Load_Glyph ( m_face,
				   charcode  ,
				   FT_LOAD_NO_SCALE  );
	if ( ft_error )
	{
		glyph->setMetaData ( GLYPH_DATA_GLYPH ,index );
		glyph->setMetaData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setMetaData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setMetaData ( GLYPH_DATA_HADVANCE ,size / scaleFactor );
		releaseFace();
		return glyph;
	}

	double takeAdvanceBeforeRender = m_glyph->metrics.horiAdvance * ( typotek::getInstance()->getDpiX() / 72.0 );
	double takeVertAdvanceBeforeRender = m_glyph->metrics.vertAdvance * ( typotek::getInstance()->getDpiX() / 72.0 );
	double takeLeftBeforeRender = ( double ) m_glyph->metrics.horiBearingX * ( typotek::getInstance()->getDpiX() / 72.0 );

// 	if(m_FTHintMode != FT_LOAD_NO_HINTING)
	{
		ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_DEFAULT | m_FTHintMode  );
	}
	// Render the glyph into a grayscale bitmap
	ft_error = FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL );
	if ( ft_error )
	{
		glyph->setMetaData ( GLYPH_DATA_GLYPH , index );
		glyph->setMetaData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setMetaData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setMetaData ( GLYPH_DATA_HADVANCE ,size  / scaleFactor );
		releaseFace();
		return glyph;
	}


	QImage img ( glyphImage() );

	if ( img.isNull() && !spaceIndex.contains ( index ) )
	{
		glyph->setMetaData ( GLYPH_DATA_GLYPH , index );
		glyph->setMetaData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setMetaData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setMetaData ( GLYPH_DATA_HADVANCE ,size /scaleFactor  );
	}
	else
	{
		glyph->setMetaData ( GLYPH_DATA_GLYPH , index );
		glyph->setMetaData ( GLYPH_DATA_BITMAPLEFT , takeLeftBeforeRender );
		glyph->setMetaData ( GLYPH_DATA_BITMAPTOP , double(m_face->glyph->bitmap_top) );
		glyph->setMetaData ( GLYPH_DATA_HADVANCE , takeAdvanceBeforeRender );
		glyph->setMetaData ( GLYPH_DATA_VADVANCE , takeVertAdvanceBeforeRender );
	}

	releaseFace();
	return glyph;
}

QImage FontItem::charImage(int charcode, double size)
{
	if(!ensureFace())
		return QImage();
	
	// Set size
	FT_Set_Char_Size ( m_face,  qRound( size  * 64 ), 0, typotek::getInstance()->getDpiX(),typotek::getInstance()->getDpiY() );
	if(FT_Load_Char( m_face, charcode , FT_LOAD_DEFAULT))
	{
		releaseFace();
		return QImage();
	}
	if(FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL ))
	{
		releaseFace();
		return QImage();
	}

	
	QImage cImg( glyphImage() );
	releaseFace();
	return cImg;
}

QImage FontItem::glyphImage(int index, double size)
{
	if(!ensureFace())
		return QImage();

	// Set size
	FT_Set_Char_Size ( m_face,  qRound( size  * 64 ), 0, typotek::getInstance()->getDpiX(),typotek::getInstance()->getDpiY() );
	if(FT_Load_Glyph( m_face, index , FT_LOAD_DEFAULT))
	{
		releaseFace();
		return QImage();
	}
	if(FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL ))
	{
		releaseFace();
		return QImage();
	}


	QImage cImg( glyphImage() );
	releaseFace();
	return cImg;
}
/// Nature line
double FontItem::renderLine ( QGraphicsScene * scene,
                              QString spec,
                              QPointF origine,
                              double lineWidth,
                              double fsize ,
			      double zindex )
{
// 	qDebug() <<fancyName() <<"::"<<"renderLine("<<scene<<spec<<lineWidth<<fsize<<zindex<<record<<")";
	double retValue ( 0.0 );
	if ( spec.isEmpty() )
		return retValue;

	ensureFace();

	double sizz = fsize;
	double scalefactor = sizz / m_face->units_per_EM;
	double pWidth = lineWidth ;
	const double distance = 20;
	QPointF pen ( origine );
	if ( m_rasterFreetype )
	{
		QList<QGraphicsPixmapItem*> mayBeRemoved;
		for ( int i=0; i < spec.length(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromCharPix ( spec.at ( i ).unicode(), sizz );
			if( spec.at(i).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}
			if ( !glyph )
			{
				continue;
			}
			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= ( glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() ) * scalefactor;
				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;

			/************************************/
			
			mayBeRemoved.append(glyph);
			
			scene->addItem ( glyph );
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue += 1;
			
			glyph->setPos ( pen.x() + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() * scalefactor, pen.y() - glyph->data ( GLYPH_DATA_BITMAPTOP ).toInt() );
			glyph->setZValue ( zindex );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			/************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else if ( m_progression == PROGRESSION_TTB )
			{
				pen.ry() +=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
			}
		}
	}
	else
	{
		QList<QGraphicsPathItem*> mayBeRemoved;
		for ( int i=0; i < spec.length(); ++i )
		{
//			if ( !scene->sceneRect().contains ( pen ) && record )
//				break;
			QGraphicsPathItem *glyph = itemFromChar ( spec.at ( i ).unicode(), sizz );
			if ( !glyph )
				continue;
			if( spec.at(i).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}
			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;

				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}

			/*********************************/
			scene->addItem ( glyph );
			glyph->setPen(Qt::NoPen);
			
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue += 1;
			
			glyph->setPos ( pen );
			glyph->setZValue ( zindex );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			/*********************************/

			if ( m_progression == PROGRESSION_LTR )
			{
				pen.rx() += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pen.ry() +=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
			}
		}
	}

	releaseFace();
	return retValue;
}

/// Featured line
double FontItem::renderLine ( OTFSet set, QGraphicsScene * scene, QString spec, QPointF origine,double lineWidth, double fsize)
{
// 	qDebug()<<"Featured("<< spec <<")";
	double retValue ( 0.0 );
	if ( spec.isEmpty() )
		return retValue;
	if ( !m_isOpenType )
		return retValue;
	ensureFace();

	otf = new FMOtf ( m_face, 0x10000 );// You think "What’s this 0x10000?", so am I! Just accept Harfbuzz black magic :)
	if ( !otf )
		return retValue;
	double sizz = fsize;
	double scalefactor = sizz / m_face->units_per_EM  ;
	double pixelAdjustX = scalefactor * ( typotek::getInstance()->getDpiX() / 72.0 );
	double pixelAdjustY = scalefactor * ( typotek::getInstance()->getDpiX() / 72.0 );
	double pWidth = lineWidth ;
	const double distance = 20;
	QList<RenderedGlyph> refGlyph = otf->procstring ( spec, set );
// 	qDebug() << "Get line "<<spec;
	delete otf;
	otf = 0;
// 	qDebug() << "Deleted OTF";
	if ( refGlyph.count() == 0 )
	{
		return 0;
	}
	QPointF pen ( origine );

	if ( m_rasterFreetype )
	{
		QList<QGraphicsPixmapItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromGindexPix ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			// Now, all is in the log!
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * pixelAdjustX;
				pWidth -= refGlyph[i].xadvance * pixelAdjustX ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * pixelAdjustY;
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * pixelAdjustX;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}

			/*************************************************/
			
			mayBeRemoved.append(glyph);
						
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * pixelAdjustX ) + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() * scalefactor  ,
			                pen.y() + ( refGlyph[i].yoffset * pixelAdjustY ) - glyph->data ( GLYPH_DATA_BITMAPTOP ).toInt() );
			/*************************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * pixelAdjustX ;
			else if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * pixelAdjustY ;
		}
	}
	else
	{
		QList<QGraphicsPathItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPathItem *glyph = itemFromGindex ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * scalefactor;
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * scalefactor;
				pWidth -=  refGlyph[i].yadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * scalefactor ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}

			/**********************************************/
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setPen(Qt::NoPen);
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * scalefactor ),
			                pen.y() + ( refGlyph[i].yoffset * scalefactor ) );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			/*******************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * scalefactor;
			if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * scalefactor;
		}
	}


	releaseFace();
	return retValue + 1;
}

/// Shaped line
double FontItem::renderLine ( QString script, QGraphicsScene * scene, QString spec, QPointF origine,double lineWidth, double fsize)
{
	qDebug()<<"Shaped("<< spec <<")";
	double retValue(0.0);
	if ( spec.isEmpty() )
		return 0;
	if ( !m_isOpenType )
		return 0;
	ensureFace();

	otf = new FMOtf ( m_face, 0x10000 );
	if ( !otf )
		return 0;
	
	FMShaperFactory *shaperfactory = 0;
	//	switch(m_shaperType)
	//	{
	//		case FMShaperFactory::FONTMATRIX : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
	//		break;
	//		case FMShaperFactory::HARFBUZZ : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::HARFBUZZ );
	//		break;
	//		case FMShaperFactory::ICU : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::ICU );
	//		break;
	//		case FMShaperFactory::M17N : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::M17N );
	//		break;
	//		case FMShaperFactory::PANGO : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::PANGO );
	//		break;
	//		case FMShaperFactory::OMEGA : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::OMEGA);
	//		break;
	//		default : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
	//	}

	/// Let's do it only with ICU atm.
	shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::ICU );

	GlyphList refGlyph ( shaperfactory->doShape( spec ) );
	delete shaperfactory;

	double sizz = fsize;
	double scalefactor = sizz / m_face->units_per_EM  ;
	double pixelAdjustX = scalefactor * ( typotek::getInstance()->getDpiX() / 72.0 );
	double pixelAdjustY = scalefactor * ( typotek::getInstance()->getDpiX() / 72.0 );
	double pWidth = lineWidth ;
	const double distance = 20;


// 	qDebug() << "Get line "<<spec;
	delete otf;
	otf = 0;
// 	qDebug() << "Deleted OTF";
	if ( refGlyph.count() == 0 )
	{
		return 0;
	}
	QPointF pen ( origine );

	if ( m_rasterFreetype )
	{
		QList<QGraphicsPixmapItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromGindexPix ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * pixelAdjustX;
				pWidth -= refGlyph[i].xadvance * pixelAdjustX ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * pixelAdjustY;
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * pixelAdjustX;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}

			/*************************************************/
			
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * pixelAdjustX ) + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() * scalefactor  ,
			                pen.y() - ( refGlyph[i].yoffset * pixelAdjustY ) - glyph->data ( GLYPH_DATA_BITMAPTOP ).toInt() );
			/*************************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * pixelAdjustX ;
			else if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * pixelAdjustY ;
		}
	}
	else
	{
		QList<QGraphicsPathItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPathItem *glyph = itemFromGindex ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * scalefactor;
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * scalefactor;
				pWidth -=  refGlyph[i].yadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * scalefactor ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						delete rm;
					}
					
					break;
				}
			}

			/**********************************************/
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setPen(Qt::NoPen);
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * scalefactor ),
			                pen.y() - ( refGlyph[i].yoffset * scalefactor ) );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			/*******************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * scalefactor;
			if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * scalefactor;
		}
	}


	releaseFace();
	return retValue + 1;
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



int FontItem::firstChar()
{
	if(!ensureFace())
		return 0;
	
	FT_UInt anIndex (1);
	FT_UInt fc ( FT_Get_First_Char ( m_face, &anIndex ));
	releaseFace();
	
	return fc;
}

int FontItem::lastChar()
{
	if(!ensureFace())
		return 0;
	
	FT_UInt index (1);
	FT_UInt cc =  FT_Get_First_Char ( m_face, &index );
	int lc(0);
	while ( index )
	{
		lc = cc;
		cc = FT_Get_Next_Char ( m_face, cc, &index );
	}
	
	releaseFace();
	return lc ;
}

int FontItem::countChars()
{
	if(!ensureFace())
		return 0;
	
	FT_UInt index (1);
	FT_UInt cc =  FT_Get_First_Char ( m_face, &index );
	int n(0);
	while ( index )
	{
		++n;
		cc = FT_Get_Next_Char ( m_face, cc, &index );
	}
	
	releaseFace();
	return n;
}

int FontItem::nextChar(int from, int offset)
{
	if(!ensureFace())
		return 0;
	
	FT_UInt index (1);
	int cc(from);
	for ( int i(0); i < offset; ++i )
	{
		cc = FT_Get_Next_Char ( m_face, cc, &index );
	}
	
	releaseFace();
	return cc;
}

int FontItem::countCoverage ( int begin_code, int end_code )
{
	if(!ensureFace())
		return 0;
// 	qDebug()<<"CC B E"<<begin_code<<end_code;
	FT_ULong  charcode = begin_code ;
	FT_UInt   gindex = 0;
	int count = 0;
	if ( begin_code >= 0 )
	{
		for ( ;charcode <= end_code ; ++charcode)
		{
			if( FT_Get_Char_Index ( m_face, charcode))
				++count;
		}
	}
	else
	{
		FT_UInt anIndex = 0;
		count = m_numGlyphs;
		FT_UInt anyChar =  FT_Get_First_Char ( m_face, &anIndex );
		while ( anIndex )
		{
			anyChar =  FT_Get_Next_Char ( m_face,anyChar,&anIndex );
			if ( anIndex )
				--count;
		}
	}
	releaseFace();
	return count;//something weird with freetype which put a valid glyph at the beginning of each lang ??? Or a bug here...
}

bool FontItem::hasCharcode(int cc)
{
	if(!ensureFace())
		return false;
	bool ret(true);
	if( !FT_Get_Char_Index( m_face, cc ) )
	{
		ret = false;
	}
	releaseFace();
	return ret;
}

bool FontItem::hasChars(const QString & s)
{
	if(!ensureFace())
		return false;
	bool ret(true);
	
	foreach(QChar c, s)
	{
		if( !FT_Get_Char_Index( m_face, c.unicode() ) )
		{
			ret = false;
			break;
		}
	}
	
	releaseFace();
	return ret;
}

void FontItem::renderAll ( QGraphicsScene * scene , int begin_code, int end_code )
{

	ensureFace();

	FMGlyphsView *allView(0);
	if(scene->views().count() > 0)
		allView = reinterpret_cast<FMGlyphsView*> ( scene->views() [0] );
	else
	{
		releaseFace();
		return;
	}

	deRenderAll();
	if ( !allView->isVisible() )
	{
		releaseFace();
		return;
	}


	adjustGlyphsPerRow ( allView->width() );
	QRectF exposedRect ( allView->visibleSceneRect() );
//        qDebug() << exposedRect;

	double leftMargin = ( ( exposedRect.width() - ( 100 * m_glyphsPerRow ) ) / 2 ) + 30;
	double aestheticTopMargin = 12;
	QPointF pen ( leftMargin, 50  + aestheticTopMargin );

	int nl = 0;

	FT_ULong  charcode;
	FT_UInt   gindex = 1;
	double sizz = 50;
	charcode = begin_code;
	QPen selPen ( Qt::gray );
	
	QFont infoFont ( typotek::getInstance()->getChartInfoFontName() , typotek::getInstance()->getChartInfoFontSize() );
	QBrush selBrush ( QColor ( 255,255,255,0 ) );
	QColor txtColor(60,60,60,255);
	if ( begin_code >= 0 )
	{
		if ( m_isEncoded )
		{
			while ( charcode <= end_code && gindex )
			{
				if ( nl == m_glyphsPerRow )
				{
					nl = 0;
					pen.rx() = leftMargin;
					pen.ry() += 100;
				}
				if ( ( pen.y() + 100 ) < exposedRect.y() || pen.y() - 100 > ( exposedRect.y() + exposedRect.height() ) )
				{
					charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
// 					qDebug() << "charcode = "<< charcode <<" ; gindex = "<< gindex;
					pen.rx() += 100;
					++nl;

					continue;
				}

				QGraphicsPathItem *pitem = itemFromChar ( charcode , sizz );
				if ( pitem )
				{
					uint ucharcode = charcode;

					scene->addItem ( pitem );
					pitem->setPen(Qt::NoPen);
					pitem->setPos ( pen );
					pitem->setData ( 1,"glyph" );
					pitem->setData ( 2,gindex );
					pitem->setData ( 3,ucharcode );
					glyphList.append ( pitem );

					pitem->setZValue ( 10 );

					QGraphicsTextItem *tit= scene->addText ( glyphName ( charcode ), infoFont );
					tit->setDefaultTextColor(txtColor);
					tit->setPos ( pen.x()-27,pen.y() + 15 );
					tit->setData ( 1,"label" );
					tit->setData ( 2,gindex );
					tit->setData ( 3,ucharcode );
					labList.append ( tit );
					tit->setZValue ( 1 );

					QGraphicsTextItem *tit2= scene->addText ( "U+" + QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) )  +" ("+ QString::number ( charcode ) +")"  , infoFont );
					tit2->setDefaultTextColor(txtColor);
					tit2->setPos ( pen.x()-27,pen.y() + 28 );
					tit2->setData ( 1,"label" );
					tit2->setData ( 2,gindex );
					tit2->setData ( 3,ucharcode );
					labList.append ( tit2 );
					tit2->setZValue ( 1 );

					QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
					rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
					rit->setData ( 1,"select" );
					rit->setData ( 2,gindex );
					rit->setData ( 3,ucharcode );
					rit->setZValue ( 100 );
					selList.append ( rit );

					pen.rx() += 100;
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
					pen.rx() = leftMargin;
					pen.ry() += 100;
				}

				if ( ( pen.y() + 100 ) < exposedRect.y() || pen.y() - 100 > ( exposedRect.y() + exposedRect.height() ) )
				{
					++charcode;
					++nl;

					continue;
				}

				QGraphicsPathItem *pitem = itemFromGindex ( charcode , sizz );
				if ( pitem )
				{
					scene->addItem ( pitem );
					pitem->setPos ( pen );
					pitem->setData ( 1,"glyph" );
					pitem->setData ( 2,gindex );
					pitem->setData ( 3,0 );
					glyphList.append ( pitem );
					pitem->setZValue ( 10 );

					QGraphicsTextItem *tit= scene->addText ( QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) ) , infoFont );
					tit->setDefaultTextColor(txtColor);
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
		// 1/ what is "out charmap"?
		FT_UInt anIndex = 1;
		QList<bool> notCovered;
		for ( int i=1; i  < m_numGlyphs +1; ++i )
			notCovered << true;
		FT_UInt anyChar =  FT_Get_First_Char ( m_face, &anIndex );
		while ( anIndex )
		{
			anyChar =  FT_Get_Next_Char ( m_face,anyChar,&anIndex );
			if ( anIndex && (anIndex <= m_numGlyphs))
			{
				notCovered[anIndex] = false;
			}
		}

		// 2/ fill with glyphs
		for ( int i = 1; i < notCovered.count(); ++i )
		{
			if ( !notCovered[i] )
				continue;
			if ( nl == m_glyphsPerRow )
			{
				nl = 0;
				pen.rx() = leftMargin;
				pen.ry() += 100;
			}

			if ( ( pen.y() + 100 ) < exposedRect.y() || pen.y() - 100 > ( exposedRect.y() + exposedRect.height() ) )
			{
				++nl;

				continue;
			}

			QGraphicsPathItem *pitem = itemFromGindex ( i , sizz );
			if ( pitem )
			{
				scene->addItem ( pitem );
				pitem->setPos ( pen );
				pitem->setData ( 1,"glyph" );
				pitem->setData ( 2, i );
				pitem->setData ( 3,0 );
				glyphList.append ( pitem );
				pitem->setZValue ( 10 );

				QGraphicsTextItem *tit= scene->addText ( QString ( "I+%1" ).arg ( i ), infoFont );
				tit->setDefaultTextColor(txtColor);
				tit->setPos ( pen.x(),pen.y() + 15 );
				tit->setData ( 1,"label" );
				tit->setData ( 2,i );
				tit->setData ( 3,0 );
				labList.append ( tit );
				tit->setZValue ( 1 );
					
				QGraphicsTextItem *tit2= scene->addText ( glyphName ( i , false), infoFont );
				tit2->setDefaultTextColor(txtColor);
				tit2->setPos ( pen.x()-27,pen.y() + 30 );
				tit2->setData ( 1,"label" );
				tit2->setData ( 2,i );
				labList.append ( tit2 );
				tit2->setZValue ( 1 );
				
				QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
				rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
				rit->setData ( 1,"select" );
				rit->setData ( 2,i );
				rit->setData ( 3,0 );
				rit->setZValue ( 100 );
				selList.append ( rit );

				pen.rx() += 100;
				++nl;
			}
		}
	}

	scene->setSceneRect ( QRectF ( 0,0, m_glyphsPerRow * 100 + 30, pen.y() + 100 ) );
	allIsRendered = true;
	releaseFace();

//        scene->blockSignals(false);
//	exposedRect = allView->visibleSceneRect();
// 	qDebug() << "ENDOFRENDERALL" <<exposedRect.x() << exposedRect.y() << exposedRect.width() << exposedRect.height();
}

int FontItem::renderChart ( QGraphicsScene * scene, int begin_code, int end_code ,double pwidth, double pheight )
{
// 	qDebug() <<"FontItem::renderChart ("<< begin_code<<end_code <<")";

	ensureFace();
	int nl ( 0 );
	int retValue ( 0 );

	FT_ULong  charcode;
	FT_UInt   gindex = 1;
	double sizz = 50;
	charcode = begin_code;
	adjustGlyphsPerRow ( qRound(pwidth) );

	double leftMargin = 30 + ( ( pwidth - ( m_glyphsPerRow * 100 ) ) / 2 );
	double aestheticTopMargin = 0;
	QPointF pen ( leftMargin, sizz + aestheticTopMargin );


	QPen selPen ( Qt::gray );
	QFont infoFont (  typotek::getInstance()->getChartInfoFontName() , typotek::getInstance()->getChartInfoFontSize()  );
	QBrush selBrush ( QColor ( 255,255,255,0 ) );

	while ( charcode <= end_code && gindex )
	{
		if ( nl == m_glyphsPerRow )
		{
			nl = 0;
			pen.rx() = leftMargin;
			pen.ry() += 100;
		}
		if ( pen.y() > pheight - 30 )
		{
			releaseFace();
			return  retValue  ;
		}


		QGraphicsPathItem *pitem = itemFromChar ( charcode , sizz );
		if ( pitem )
		{
			uint ucharcode = charcode;

			scene->addItem ( pitem );
			pitem->setPos ( pen );
			pitem->setData ( 1,"glyph" );
			pitem->setData ( 2,gindex );
			pitem->setData ( 3,ucharcode );
// 			glyphList.append ( pitem );

			pitem->setZValue ( 10 );

			QGraphicsTextItem *tit= scene->addText ( glyphName ( charcode ), infoFont );
			tit->setPos ( pen.x()-27,pen.y() + 15 );
			tit->setData ( 1,"label" );
			tit->setData ( 2,gindex );
			tit->setData ( 3,ucharcode );
// 			labList.append ( tit );
			tit->setZValue ( 1 );

			QGraphicsTextItem *tit2= scene->addText ( "U+" + QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) )  +" ("+ QString::number ( charcode ) +")"  , infoFont );
			tit2->setPos ( pen.x()-27,pen.y() + 28 );
			tit2->setData ( 1,"label" );
			tit2->setData ( 2,gindex );
			tit2->setData ( 3,ucharcode );
// 			labList.append ( tit2 );
			tit2->setZValue ( 1 );

			QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
			rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
			rit->setData ( 1,"select" );
			rit->setData ( 2,gindex );
			rit->setData ( 3,ucharcode );
			rit->setZValue ( 100 );

			pen.rx() += 100;
			++nl;
			++retValue;
		}
		retValue = charcode;
		charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
	}
	releaseFace();

	return retValue ;
}


QString FontItem::glyphName ( int codepoint, bool codeIsChar )
{
	ensureFace();

	int index(0);
	
	if(codeIsChar)
	{
		index = FT_Get_Char_Index ( m_face, codepoint );
		if ( index== 0 )
		{
			return "noname";
		}
	}
	else
		index = codepoint;

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

QGraphicsPathItem * FontItem::hasCodepointLoaded ( int code )
{
	for ( int i=0;i< glyphList.count();++i )
	{
		if ( glyphList.at ( i )->data ( 3 ).toInt() == code )
			return glyphList.at ( i );
	}
	return 0;
}


QPixmap FontItem::oneLinePreviewPixmap ( QString oneline , QColor fg_color, QColor bg_color, int size_w , int size_f )
{
//	if ( m_remote )
//		return fixedPixmap;
//	if ( !theOneLinePreviewPixmap.isNull() )
//	{
//		if ( theOneLinePreviewPixmap.width() == size_w )
//			return theOneLinePreviewPixmap;
//	}
	if ( !ensureFace() )
		return QPixmap();
	QRectF savedRect = theOneLineScene->sceneRect();

	double theSize = (size_f == 0) ? typotek::getInstance()->getPreviewSize() : size_f;
	double pt2px = typotek::getInstance()->getDpiX() / 72.0;
	double theHeight = theSize * 1.3 * pt2px;
	double theWidth;
	if ( size_w == 0 )
	{
		theWidth = theSize * pt2px * oneline.count() * 1.2;
	}
	else
	{
		theWidth = size_w;
	}
// 	qDebug() << theSize << theHeight << theWidth;
	theOneLineScene->setSceneRect ( 0,0,theWidth, theHeight );
	bool pRTL = typotek::getInstance()->getPreviewRTL();
	QPointF pen ( pRTL ? theWidth - 16 : 16 , theSize *  pt2px );

	int fsize = qRound(theSize) * 64  ;
	double scalefactor = theSize / m_face->units_per_EM;

	QPixmap linePixmap ( qRound(theWidth), qRound(theHeight) );
	linePixmap.fill ( bg_color );
	QPainter apainter ( &linePixmap );

	bool canRender(true);
	for ( int i(0);i < oneline.count() ; ++i )
	{
		if(FT_Get_Char_Index ( m_face, oneline[i].unicode() ) == 0)
		{
			canRender = false;
			break;
		}
	}
	if(canRender)
	{
		for ( int i(0);i < oneline.count() ; ++i )
		{
			int glyphIndex = FT_Get_Char_Index ( m_face, oneline[i].unicode() );

			FT_Set_Char_Size ( m_face,
					   fsize,
					   0,
					   typotek::getInstance()->getDpiX(),
					   typotek::getInstance()->getDpiY() );

			if(FT_Load_Glyph ( m_face, glyphIndex, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING ) > 0)
				continue;
			if(FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL ) > 0)
				continue;

			if ( pRTL )
				pen.rx() -= qRound ( double(m_glyph->linearHoriAdvance) / 65536 );
			apainter.drawImage ( pen.x() +  m_glyph->bitmap_left , pen.y() - m_glyph->bitmap_top , glyphImage(fg_color) );
			if ( !pRTL )
				pen.rx() += qRound ( double(m_glyph->linearHoriAdvance) / 65536 );
		}
	}
	else
	{
		QString cantRenderString(tr("(%1)", "when doing the font preview, used to denote a font that can not displayed its name"));
		apainter.drawText(pen.x(),pen.y(), cantRenderString.arg(oneline));
	}

	apainter.end();
	releaseFace();

	return linePixmap;
//	theOneLinePreviewPixmap = linePixmap;


//	if ( !theOneLinePreviewPixmap.isNull() )
//		return theOneLinePreviewPixmap;

//	theOneLinePreviewPixmap = QPixmap ( qRound(theWidth), qRound(theHeight) );
//	theOneLinePreviewPixmap.fill ( Qt::lightGray );
//	return theOneLinePreviewPixmap;
}

void FontItem::clearPreview()
{
//	if ( m_remote )
//		return;
//	if ( !theOneLinePreviewPixmap.isNull() )
//		theOneLinePreviewPixmap = QPixmap();
}


FontInfoMap FontItem::moreInfo()
{
	FontInfoMap ret;
	if(!ensureFace())
		return ret;
	
	if ( testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" )
	{
		m_isOpenType = true;
	}
	
	if(m_isOpenType)
	{
		ret =  moreInfo_sfnt();
	}
	else
	{
		ret =  moreInfo_type1();
	}
	releaseFace();
	return ret;
}

QString FontItem::panose()
{
	if(!ensureFace())
		return QString("0:0:0:0:0:0:0:0:0:0");
	QStringList pl;
	TT_OS2 *os2 = static_cast<TT_OS2*> ( FT_Get_Sfnt_Table ( m_face, ft_sfnt_os2 ) );
	if ( os2 )
	{
		for ( int bI ( 0 ); bI < 10; ++bI )
		{
			pl << QString::number ( os2->panose[bI] ) ;
		}
	}
	else
	{
		for ( int bI ( 0 ); bI < 10; ++bI )
		{
			pl << QString::number ( 0 ) ;
		}
	}
	releaseFace();
	return pl.join(":");
}

QStringList FontItem::supportedLangDeclaration()
{
	QStringList ret;
	if ( !ensureFace() )
		return ret;

	TT_OS2 *os2 = static_cast<TT_OS2*> ( FT_Get_Sfnt_Table ( m_face, ft_sfnt_os2 ) );
	if ( os2 )
	{
		QList<FT_ULong> uMaskList;
		uMaskList << os2->ulUnicodeRange1
		<< os2->ulUnicodeRange2
		<< os2->ulUnicodeRange3
		<< os2->ulUnicodeRange4;
		const QMap<int, QPair<int,int> >& uranges(FMEncData::Os2URanges()); 
		unsigned int mask(1);
		for( int i(0); i < uMaskList.count(); ++i )
		{
			for(int j(0); j < 32; ++j)
			{
				unsigned int set(mask << j);
				if((set & uMaskList[i]) > 0)
				{
					int pos((i * 32) + j);
					if(uranges.contains(pos))
					{
						QString b( FMUniBlocks::block(uranges[pos]) );
						if(!b.isEmpty())
							ret << b;
					}
				}
			}
		}
	}
	releaseFace();
	return ret;
}

double FontItem::italicAngle()
{
	double ret(0);
	if(!ensureFace())
		return ret;
	if ( testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" )
	{
		TT_Postscript *post = static_cast<TT_Postscript*> ( FT_Get_Sfnt_Table ( m_face, ft_sfnt_post ) );
		if ( post )
			ret = ( double(post->italicAngle) / double (0x10000) ) ;
	}
	else
	{
		PS_FontInfoRec sinfo ;
		int err = FT_Get_PS_Font_Info ( m_face,&sinfo );
		if ( !err )
			ret = sinfo.italic_angle;
	}

	releaseFace();
	return ret;
}

FontItem::FsType FontItem::getFsType()
{
	// After some thinking, it appears that it would be a nonsense to not retrieve it from the actual font file.
	FsType fst( NOT_RESTRICTED );
	if(!ensureFace())
		return fst;
	
	TT_OS2 *os2 = static_cast<TT_OS2*> ( FT_Get_Sfnt_Table ( m_face, ft_sfnt_os2 ) );
	
	if ( os2 )
	{
		fst = FsType(os2->fsType);
	}

	releaseFace();
	return fst;
}

int FontItem::table(const QString & tableName)
{
	if(!ensureFace())
		return 0;
	
	if ( !FT_IS_SFNT ( m_face ) )
	{
		releaseFace();
		return 0;
	}
	
	uint tag(OTF_name_tag(tableName));
	FT_ULong length( 0 );
	FT_Load_Sfnt_Table ( m_face, tag , 0, NULL, &length );
	
	releaseFace();
	
	return int(length);
}

QByteArray FontItem::tableData(const QString & tableName)
{
	QByteArray ret;
	if(!ensureFace())
		return ret;
	
	if ( !FT_IS_SFNT ( m_face ) )
	{
		releaseFace();
		return ret;
	}
	
	uint tag(OTF_name_tag(tableName));
	FT_ULong length( 0 );
	if ( !FT_Load_Sfnt_Table ( m_face, tag, 0, NULL, &length ) )
	{
		if ( length > 0 )
		{
			ret.resize ( length );
			FT_Load_Sfnt_Table ( m_face, tag, 0, ( FT_Byte * ) ret.data (), &length );
		}
	}
	releaseFace();
	return ret;
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
FontInfoMap FontItem::moreInfo_sfnt()
{
	if ( !ensureFace() )
		return FontInfoMap();

	FontInfoMap moreInfo;
	FT_SfntName tname;


	int tname_count = FT_Get_Sfnt_Name_Count ( m_face );


	//TODO check encodings and platforms
	for ( int i=0; i < tname_count; ++i )
	{
		FT_Get_Sfnt_Name ( m_face,i,&tname );
		int akey;
		if ( tname.name_id >  255 )
		{
// 			qDebug() << name() <<" has vendor’s specific name id ->" << tname.name_id;
			if ( tname.string_len > 0 )
			{
// 				akey = "VendorKey_" + QString::number ( tname.name_id );
				akey = tname.name_id ;
			}
			else
			{
				continue;
			}

		}
		else if ( tname.name_id <= FontStrings::Names().count())
		{
			akey =  tname.name_id ;
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
		// from  Pajarico, pajarico chez gmail point com
		else if ( tname.platform_id == TT_PLATFORM_APPLE_UNICODE  && tname.encoding_id == TT_APPLE_ID_UNICODE_2_0 )
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "UTF-16" );
			avalue = codec->toUnicode ( array );
		}
		else if ( tname.platform_id == TT_PLATFORM_MACINTOSH   /*&& tname.encoding_id == TT_MAC_ID_TRADITIONAL_CHINESE*/ )
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "Apple Roman" );
			avalue = codec->toUnicode ( array );
		}
		else
		{
			avalue = "Unexpected platform - encoding pair ("
			         + QString::number ( tname.platform_id )
			         + "," + QString::number ( tname.encoding_id )
			         + ")\nPlease contact Fontmatrix team.\nRun Fontmatrix in console to see more info.\nPlease, if possible, provide a font file to test.";

			qDebug() << m_name
			<< "platform_id("
			<< tname.platform_id
			<<") - encoding_id("
			<< tname.encoding_id
			<<") - "
			<< QString::number ( tname.language_id )
					<< FMEncData::LangIdMap()[tname.language_id];
		}


		if ( !avalue.isEmpty() )
		{
			moreInfo[tname.language_id][akey] = avalue;
		}
	}

	// Is there an OS/2 table?
// 	TT_OS2 *os2 = static_cast<TT_OS2*> ( FT_Get_Sfnt_Table ( m_face, ft_sfnt_os2 ) );
// 	if ( os2 /* and  wantAutoTag*/ )
// 	{
// 		// PANOSE
// 		QStringList pl;
// 		for ( int bI ( 0 ); bI < 10; ++bI )
// 		{
// 			
// 			pl << QString::number ( os2->panose[bI] ) ;
// 		}
// 
// 		moreInfo[0][FMFontDb::Panose] = pl.join(":");
// 		// FSTYPE (embedding status)
// 		if(!os2->fsType)
// 			m_OSFsType = NOT_RESTRICTED;
// 		else
// 		{
// 			if(os2->fsType & RESTRICTED)
// 				m_OSFsType |= RESTRICTED;
// 			if(os2->fsType & PREVIEW_PRINT)
// 				m_OSFsType |= PREVIEW_PRINT;
// 			if(os2->fsType &  EDIT_EMBED)
// 				m_OSFsType |= EDIT_EMBED;
// 			if(os2->fsType & NOSUBSET)
// 				m_OSFsType |= NOSUBSET;
// 			if(os2->fsType & BITMAP_ONLY)
// 				m_OSFsType |=  BITMAP_ONLY;
// 		}
// 
// 	}

	releaseFace();
	return moreInfo;
}

QString FontItem::getAlternateFamilyName()
{
	if ( !ensureFace() )
		return QString();

	FT_SfntName tname;
	int tname_count = FT_Get_Sfnt_Name_Count ( m_face );
	for ( int i=0; i < tname_count; ++i )
	{
		FT_Get_Sfnt_Name ( m_face , i , &tname );
		if ( tname.name_id == 1 && tname.language_id == 0 )
		{
			return QString ( QByteArray ( ( const char* ) tname.string, tname.string_len ) );
		}

	}
	
	releaseFace();
	return QString();
}

QString FontItem::getAlternateVariantName()
{
	if ( !ensureFace() )
		return QString();

	FT_SfntName tname;
	int tname_count = FT_Get_Sfnt_Name_Count ( m_face );
	for ( int i=0; i < tname_count; ++i )
	{
		FT_Get_Sfnt_Name ( m_face , i , &tname );
		if ( tname.name_id == 2 && tname.language_id == 0 )
		{
			return QString ( QByteArray ( ( const char* ) tname.string, tname.string_len ) );
		}

	}

	releaseFace();
	return QString();
}


FontInfoMap FontItem::moreInfo_type1()
{
	if ( !ensureFace() )
		return FontInfoMap();

	FontInfoMap moreInfo;
	PS_FontInfoRec sinfo ;
	int err = FT_Get_PS_Font_Info ( m_face,&sinfo );
	if ( err )
	{
		qDebug() <<"FT_Get_PS_Font_Info("<< m_name <<")"<<" failed :" << err;
		return FontInfoMap();
	}

	moreInfo[0][1] = sinfo.family_name;
	moreInfo[0][2] = sinfo.weight;
	moreInfo[0][4] = sinfo.full_name;
	moreInfo[0][5] = sinfo.version;
	moreInfo[0][10] = sinfo.notice;

	releaseFace();
	return moreInfo;
}

QStringList FontItem::tags()const 
{
	return FMFontDb::DB()->getValue(m_path, FMFontDb::Tags).toStringList();
}

void FontItem::addTag(const QString & t)
{
	FMFontDb::DB()->addTag(m_path, t);
}

void FontItem::setTags ( QStringList l )
{

	FMFontDb::DB()->setTags(m_path, l);
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

bool FontItem::isActivated() const
{
// 	if ( FMFontDb::DB()->getValue(m_path,FMFontDb::Activation ).toInt() > 0 )
// 		return true;
// 	
	return m_active;
}

void FontItem::setActivated ( bool act )
{
	m_active = act;
	if ( act )
	{
		FMFontDb::DB()->setValue(m_path, FMFontDb::Activation , 1);
	}
	else
	{
		FMFontDb::DB()->setValue(m_path, FMFontDb::Activation , 0);
	}
}



FMOtf * FontItem::takeOTFInstance()
{
	ensureFace();
	if ( m_isOpenType )
		otf = new FMOtf ( m_face );
	return otf;

	// It is a case where we don’t release face, thr caller have to call releaseOTFInstance;
}

void FontItem::releaseOTFInstance ( FMOtf * rotf )
{
	if ( rotf == otf )
	{
		delete otf;
		otf = 0;
	}
	releaseFace();
}

QStringList FontItem::features()
{
	QStringList ret;
	if(!takeOTFInstance())
	{
		releaseOTFInstance(otf);
		return ret;
	}
	
	foreach ( QString table, otf->get_tables() )
	{
		otf->set_table ( table );
		foreach ( QString script, otf->get_scripts() )
		{
			otf->set_script ( script );
			foreach ( QString lang, otf->get_langs() )
			{
				otf->set_lang ( lang );
				foreach ( QString feature, otf->get_features() )
				{
					if(ret.contains(feature))
						ret << feature;
				}
			}
		}
	}
	releaseOTFInstance(otf);
	return ret;
}

int FontItem::showFancyGlyph ( QGraphicsView *view, int charcode , bool charcodeIsAGlyphIndex )
{
	ensureFace();

	int ref ( fancyGlyphs.count() );
	QRect allRect ( view->rect() );
	QRect targetRect ( view->mapToScene ( allRect.topLeft() ).toPoint(),  view->mapToScene ( allRect.bottomRight() ).toPoint() ) ;
// 	qDebug() <<  allRect.topLeft() << view->mapToScene ( allRect.topLeft() );

	// We’ll try to have a square subRect that fit in view ;-)
	int squareSideUnit = qMin ( allRect.width() * 0.1,  allRect.height() * 0.1) ;
	int squareSide = 8 * squareSideUnit;
	int squareXOffset = ( allRect.width() - squareSide ) / 2;
	int squareYOffset = ( allRect.height() - squareSide ) / 2;
	QRect subRect ( QPoint ( squareXOffset , squareYOffset ),
	                QSize ( squareSide, squareSide ) );
	QPixmap pix ( allRect.width(), allRect.height() );
	pix.fill ( QColor ( 30,0,0,120 ) );
	QPainter painter ( &pix );
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush ( Qt::white );
	painter.setPen ( QPen ( QBrush( QColor ( 0,0,0,255 ) ), 3/*, Qt::DashLine*/ ) );
	painter.drawRoundRect ( subRect,5,5 );
	painter.setPen ( QPen ( QColor ( 0,0,255,120 ) ) );

	ft_error = FT_Set_Pixel_Sizes ( m_face, 0, qRound(subRect.height() * 0.8) );
	if ( ft_error )
	{
		return -1;
	}
	if ( !charcodeIsAGlyphIndex )
		ft_error = FT_Load_Char ( m_face, charcode  , FT_LOAD_RENDER );
	else
		ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_RENDER );
	if ( ft_error )
	{
		return -1;
	}

	QVector<QRgb> palette;
	for ( int i = 0; i < m_face->glyph->bitmap.num_grays; ++i )
	{
		palette << qRgb ( 255 - i,255 - i,255 - i );
	}
	QImage img ( m_face->glyph->bitmap.buffer,
	             m_face->glyph->bitmap.width,
	             m_face->glyph->bitmap.rows,
	             m_face->glyph->bitmap.pitch,
	             QImage::Format_Indexed8 );
	img.setColorTable ( palette );

	double scaledBy = 1.0;
	if ( img.width() > subRect.width() )
	{
		scaledBy = ( double ) subRect.width() / ( double ) img.width() * 0.8;
// 		qDebug() <<"scaledBy = " << scaledBy ;
		img = img.scaledToWidth ( qRound(subRect.width() * 0.8 ),Qt::SmoothTransformation );

	}

	QPoint gPos ( subRect.topLeft() );
	gPos.rx() += ( subRect.width() - img.width() ) / 2;
	gPos.ry() += ( subRect.height() - img.height() ) /2;
	painter.drawImage ( gPos, img );


	/// Draw metrics
	int iAngle(italicAngle());
	QPoint pPos ( gPos );
	pPos.rx() -= qRound(m_face->glyph->bitmap_left * scaledBy);
	pPos.ry() += qRound(m_face->glyph->bitmap_top * scaledBy);
	double aF(tan((3.14/180.0) * iAngle));
	double asc(subRect.top() - pPos.y());
	double desc(pPos.y() - subRect.bottom());
	//left
	painter.drawLine ( pPos.x() + (asc * aF), subRect.top(),
			   pPos.x() - (desc * aF), subRect.bottom() );
	//right
	painter.drawLine (  qRound(pPos.x() + m_face->glyph->metrics.horiAdvance / 64 * scaledBy ) + (asc * aF), subRect.top(),
			    qRound( pPos.x() + m_face->glyph->metrics.horiAdvance / 64 * scaledBy) - (desc * aF), subRect.bottom() );
	//baseline
	painter.drawLine ( subRect.left() , pPos.y() ,
			   subRect.right(),  pPos.y() );

	painter.end();

	QGraphicsPixmapItem *fancyGlyph = new  QGraphicsPixmapItem;
	fancyGlyph->setPixmap ( pix );
	fancyGlyph->setZValue ( 10000 );
	fancyGlyph->setPos ( targetRect.topLeft() );
	view->scene()->addItem ( fancyGlyph );
	fancyGlyphs[ref] =  fancyGlyph ;


	QGraphicsTextItem *textIt = new QGraphicsTextItem;
	textIt->setTextWidth ( allRect.width() );

	QString itemNameStyle ( "background-color:#000;color:#fff;font-weight:bold;font-size:13pt;padding:0 3px;" );
	QString itemValueStyle ( "background-color:#fff;color:#000;font-size:9pt;padding:0 3px;" );

	if ( charcodeIsAGlyphIndex )
	{
		QString html(QString("<span style=\"%1\"> %2 </span> <span style=\"%3\"> - Index %4 <span>")
			     .arg(itemNameStyle)
			     .arg(glyphName(charcode))
			     .arg(itemValueStyle)
			     .arg(QString::number ( charcode )));
		textIt->setHtml ( html );
	}
	else
	{
		QString catString;
		catString = FontStrings::UnicodeCategory(QChar::category(static_cast<uint> ( charcode )));

		QString html(QString("<span style=\"%1\"> %2 </span> <span style=\"%3\"> %4 - U+%5  &#60;&#38;#%6;&#62; <span>")
			     .arg(itemNameStyle)
			     .arg(glyphName(charcode))
			     .arg(itemValueStyle)
			     .arg(catString)
			     .arg(QString("%1").arg(charcode, 4, 16, QChar('0')).toUpper())
			     .arg(charcode));

		textIt->setHtml ( html );
	}

// 	qDebug()<< textIt->toHtml();
//	QPointF tPos ( subRect.left() + 18.0 , subRect.bottom() );
	QRectF tRect(textIt->boundingRect());
	QPointF tPos ( -3, targetRect.bottom() - tRect.height() +5);
	textIt->setPos ( tPos );
	textIt->setZValue ( 2000000 );
	textIt->setEnabled ( true );
	textIt->setFlags ( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable );
	textIt->setData ( 10, "FancyText" );
	view->scene()->addItem ( textIt );
	fancyTexts[ref] = textIt ;


	// Alternates
	if ( !charcodeIsAGlyphIndex && m_isOpenType )
	{
		QList<int> alts ( getAlternates ( charcode ) );
		qDebug() << "PALTS"<<alts;
		double altSize ( squareSide / 6 );
		double altXOffset ( subRect.top() + 10 );
		for ( int a ( 0 ); a < alts.count() ; ++a )
		{
			QGraphicsPixmapItem *gpi ( itemFromGindexPix ( alts.at ( a ), altSize ) );
			fancyAlternates[ref] << gpi;

			QImage altI ( gpi->pixmap().toImage().alphaChannel() );
			QPixmap altP ( altI.width() * 2, altI.height() * 2 );
			altP.fill ( Qt::transparent );
			QPainter altPainter ( &altP );
			altPainter.setRenderHint ( QPainter::Antialiasing,true );
			altPainter.setBrush ( Qt::black );
			altPainter.drawRoundRect ( 5,
			                           5,
			                           altP.width() - 10,
			                           altP.height()  - 10,
			                           20,20 );
			altPainter.drawImage ( qRound(altI.width() / 2.0), qRound(altI.height() / 2.0) , altI );

			gpi->setPixmap ( altP );


			view->scene()->addItem ( gpi );
			gpi->setPos ( view->mapToScene ( subRect.right()  ,altXOffset ) );
			altXOffset += altP.height();
			gpi->setZValue ( 9999999 );
			qDebug() <<gpi->pos() <<gpi->scenePos();
		}

	}

	releaseFace();
	return ref;

}

void FontItem::hideFancyGlyph ( int ref )
{
	if ( fancyGlyphs.contains ( ref ) )
	{
		QGraphicsPixmapItem *it = fancyGlyphs.value ( ref );
		it->scene()->removeItem ( it );
		fancyGlyphs.remove ( ref );
		delete it;

	}
	if ( fancyTexts.contains ( ref ) )
	{
		QGraphicsTextItem *it = fancyTexts.value ( ref );
		it->scene()->removeItem ( it );
		fancyTexts.remove ( ref );
		delete it;
	}
	if ( fancyAlternates.value ( ref ).count() )
	{
		QList<QGraphicsPixmapItem*> pil ( fancyAlternates.value ( ref ) );
		for ( int pidx ( 0 ); pidx < pil.count(); ++pidx )
		{
			QGraphicsPixmapItem *it = pil.at ( pidx ) ;
			it->scene()->removeItem ( it );
			delete it;
		}
		fancyAlternates.remove ( ref );
	}
}


bool FontItem::isLocal()
{
	QString shem = m_url.scheme();
	if ( shem.isEmpty() || shem == "file" )
		return true;
	return false;
}

/// We don’t want to download fonts yet. We just want something to fill font tree
void FontItem::fileRemote ( QString f , QString v, QString t, QString i, QPixmap p )
{
	m_family = f;
	m_variant = v;
	m_type = t;
// 	m_cacheInfo = i;
//	fixedPixmap = p;
}

/// the same, but just for speedup startup with a lot of font files
void FontItem::fileLocal ( QString f, QString v, QString t, QString p )
{
	m_family = f;
	m_variant = v;
	m_type = t;
}

void FontItem::fileLocal ( FontLocalInfo fli )
{
	m_family = fli.family;
	m_variant = fli.variant;
	m_type = fli.type;
// 	m_panose = fli.panose;
// 	moreInfo = fli.info;
// 	if ( !fli.panose.isEmpty() )
// 	{
// 		for ( int bI ( 0 ); bI < 10; ++bI )
// 		{
// 			panoseInfo[ panoseKeys[bI] ] = panoseMap.value ( panoseKeys[bI] ).value ( m_panose.mid ( bI,1 ).toInt() ) ;
// 
// 		}
// 
// 	}

}


/// Finally, we have to download the font file
int FontItem::getFromNetwork()
{
	qDebug() <<"FontItem::getFromNetwork()";
	if ( remoteCached )
		return 1;
	if ( stopperDownload )
		return 2;
	else
		stopperDownload = true;

	QUrl url ( m_path );
	remoteHerePath = typotek::getInstance()->remoteTmpDir() + QDir::separator() + QFileInfo ( url.path() ).fileName();

	rFile = new QFile ( remoteHerePath );
	if ( !rFile->open ( QIODevice::WriteOnly ) )
	{
		qDebug() << "Can’t open " << remoteHerePath;
		delete rFile;
// 		return false;
	}

	rHttp = new QHttp ( url.host() );
	qDebug() << "Init progress Dialog";
	rProgressDialog = new QProgressDialog ( typotek::getInstance() );
	rProgressDialog->setWindowTitle ( tr ( "Fontmatrix - Download" ) );
	rProgressDialog->setLabelText ( tr ( "Downloading %1." ).arg ( m_path ) );
	rProgressDialog->show();
	rProgressDialog->raise();
	rProgressDialog->activateWindow();
	qDebug() <<"Progress dialog done";

	connect ( rHttp,SIGNAL ( dataReadProgress ( int, int ) ),this,SLOT ( slotDowloadProgress ( int,int ) ) );
	connect ( rHttp,SIGNAL ( requestFinished ( int, bool ) ),this,SLOT ( slotDownloadEnd ( int, bool ) ) );
	connect ( rHttp,SIGNAL ( done ( bool ) ),this,SLOT ( slotDownloadDone ( bool ) ) );
	connect ( rHttp,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotDownloadState ( int ) ) );

	remoteId = rHttp->get ( url.path() , rFile );
	return 2;
}

void FontItem::slotDownloadStart ( int id )
{
// 	rProgressDialog->show();
	if ( id != remoteId )
	{
		qDebug() << "catched a weird request : " << id;
	}
}

void FontItem::slotDowloadProgress ( int done, int total )
{
	rProgressDialog->setMaximum ( total );
	rProgressDialog->setValue ( done );
	qDebug() << " [" <<done << "/"<< total<<"]" ;
}

void FontItem::slotDownloadEnd ( int id, bool error )
{
	qDebug() << m_path << "::slotDownloadEnd ["<< id <<"] when remoteCached = "<< remoteCached;
	if ( id != remoteId )
	{
		qDebug() << "WTF this id("<< id <<") comes from nowhere, our is "<< remoteId;
		return;
	}
	if ( remoteCached )
	{
		qDebug() << "Youre a bit late dude.";
		return;
	}
	else
	{
		remoteCached = true;
	}
	rFile->flush();
	rFile->close();
	rHttp->close();

	delete rProgressDialog;
	delete rFile;

	emit dowloadFinished();
}

void FontItem::slotDownloadDone ( bool error )
{
	qDebug() << "slotDownloadDone(" <<error<<")";
}

void FontItem::slotDownloadState ( int state )
{
// 	qDebug() << "slotDownloadState("<<state<<")";
	if ( state == QHttp::Unconnected  && rHttp )
	{
		qDebug() << "slotDownloadState( QHttp::Unconnected )";
		delete rHttp;
		rHttp = 0;
	}

}


void FontItem::trimSpacesIndex()
{
	if ( !spaceIndex.isEmpty() )
		return;
	if ( !ensureFace() )
		return;

	int gIndex ( 0 );
	for ( int i ( 0 ); i < legitimateNonPathChars.count(); ++i )
	{
		gIndex =   FT_Get_Char_Index ( m_face , legitimateNonPathChars[i] );
		if ( gIndex )
		{
// 			qDebug()<<"Space : " << legitimateNonPathChars[i] << " is : "<<gIndex;
			spaceIndex << gIndex;
		}
	}

	releaseFace();
}

QString FontItem::activationName()
{
	if ( m_remote /*|| m_lock*/ )
		return QString();

	QFileInfo fi ( m_path );
	QString prefix ( "%1-" );
	return  prefix.arg ( fi.size() ) + fi.fileName();
}

QString FontItem::activationAFMName()
{
	if ( m_remote /*|| m_lock*/ )
		return QString();
	if ( m_afm.isEmpty() )
		return QString();

	QFileInfo afi ( m_afm );
	QFileInfo fi ( m_path );
	QString prefix ( "%1-" );
	return  prefix.arg ( fi.size() ) + afi.fileName();
}

QList< int > FontItem::getAlternates ( int ccode )
{
	QList<int> ret;
	if ( !ensureFace() )
		return ret;
	if ( !otf && m_isOpenType )
	{
		otf = new FMOtf ( m_face );
		if ( !otf )
			return ret;
	}

	int glyphIndex ( FT_Get_Char_Index ( m_face, ccode ) );
	QList<OTFSet> setList;
	setList.clear();

	otf->set_table ( "GSUB" );
	foreach ( QString script, otf->get_scripts() )
	{
		otf->set_script ( script );
		foreach ( QString lang, otf->get_langs() )
		{
			otf->set_lang ( lang );
			QStringList fl ( otf->get_features() );
			if ( fl.contains ( "aalt" ) )
			{
				OTFSet set;
				set.script = script;
				set.lang = lang;
				set.gpos_features.clear();
				set.gsub_features = QStringList ( "aalt" );
				setList << set;
				qDebug() << "AALT"<<script<< lang;
			}
		}
	}

	QString spec;
	spec = QChar ( ccode );

	foreach ( OTFSet set, setList )
	{
		QList<RenderedGlyph> rendered ( otf->procstring ( spec, set ) );
		if(rendered.isEmpty())
			continue;
		if ( rendered.at ( 0 ).glyph != glyphIndex )
		{
			if ( !ret.contains ( rendered.at ( 0 ).glyph ) )
				ret << rendered.at ( 0 ).glyph;
		}
		if ( !otf->altGlyphs.isEmpty() )
		{
			QList<int> l ( otf->altGlyphs );
			foreach ( int g, l )
			{
				if ( !ret.contains ( g ) && g != glyphIndex )
					ret << g;
			}
		}
	}

	delete otf;
	otf = 0;

	releaseFace();
	return ret;
}

QImage FontItem::glyphImage(QColor color)
{
	QImage img ( m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, QImage::Format_Indexed8);
// 	QImage img ( m_face->glyph->bitmap.buffer,
// 	             m_face->glyph->bitmap.width,
// 	             m_face->glyph->bitmap.rows,
// 	             m_face->glyph->bitmap.pitch,
// 	             QImage::Format_Indexed8 );
	
// 	qDebug()<<"GSlot"<< m_face->glyph->bitmap.buffer 
// 			<< m_face->glyph->bitmap.width 
// 			<< m_face->glyph->bitmap.rows 
// 			<< m_face->glyph->bitmap.pitch ;
	
	if ( (m_face->glyph->bitmap.num_grays != 256) 
		     || (color != QColor(Qt::black))  )
	{
		QVector<QRgb> palette;
		palette.clear();
		int r(color.red());
		int g(color.green());
		int b(color.blue());
		for ( int aa = 0; aa < m_face->glyph->bitmap.num_grays; ++aa )
		{
			palette << qRgba ( r,g,b, aa );
		}
		img.setColorTable ( palette );
	}
	else
	{
		img.setColorTable ( gray256Palette );
	}
	
	
	unsigned char * cursor(m_face->glyph->bitmap.buffer);
// 	QString dbs;
	for(int r(0); r < m_face->glyph->bitmap.rows; ++r)
	{
// 		dbs.clear();
		for(int x(0); x < m_face->glyph->bitmap.width; ++x)
		{
			img.setPixel( x, r, *(cursor + x));
// 			dbs += (*(cursor + x) > 0) ? "+" : ".";
		}
// 		qDebug()<<dbs;
		cursor += m_face->glyph->bitmap.pitch;
	}
	


	return img;
}


FontInfoMap  FontItem::rawInfo()
{
	return FMFontDb::DB()->getInfoMap(m_path);
}






int FontItem::shaperType() const
{
	return m_shaperType;
}


void FontItem::setShaperType ( int theValue )
{
	m_shaperType = theValue;
}

GlyphList FontItem::glyphs ( QString spec, double fsize )
{
// 	qDebug()<<"glyphs ("<< spec.left(24) <<", "<<fsize<<" )";
	FMHyphenator *hyph = typotek::getInstance()->getHyphenator();
	GlyphList ret;
	if ( spec.isEmpty() || fsize <= 0.0 )
		return ret;
	if( !ensureFace() )
		return ret;
	double scalefactor = fsize / m_face->units_per_EM  ;

	QChar spaceChar(' ');
	int startSpaceCount(0);
	int endSpaceCount(0);
	int specCount(spec.count());
	for(int s(0); s < specCount; ++s)
	{
		if(spec.at(s) == spaceChar)
			++startSpaceCount;
		else
			break;
	}
	if(startSpaceCount != specCount)
	{
		for(int s(specCount-1); s >= 0; --s)
		{
			if(spec.at(s) == spaceChar)
				++endSpaceCount;
			else
				break;
		}
	}
	
	QStringList stl(spec.split(spaceChar, QString::SkipEmptyParts));
	
	QGraphicsPathItem *glyph = itemFromChar ( spaceChar.unicode() , fsize );
	RenderedGlyph wSpace(glyph->data(GLYPH_DATA_GLYPH).toInt(),0, glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor ,0,0,0,' ',false);
	delete glyph;
	for(int s(0); s < startSpaceCount; ++s)
	{
		ret << wSpace;
	}
	for(QStringList::const_iterator sIt(stl.constBegin());sIt != stl.constEnd(); ++ sIt)
	{
		if(sIt != stl.constBegin())
		{
			ret << wSpace;
		}
		HyphList hl;
		if(hyph)
		{
			hl = hyph->hyphenate(*sIt) ;
// 			if(hl.count())qDebug()<<"Hyph W C"<<*sIt<<hl.count();
		}
		
		for ( int i ( 0 ); i < (*sIt).count();++i )
		{
			glyph = itemFromChar ( (*sIt).at ( i ).unicode(), fsize );
			if ( !glyph )
			{
				continue;
			}
			RenderedGlyph rg;
			rg.glyph = glyph->data(GLYPH_DATA_GLYPH).toInt();
			rg.log = i; // We are in a 1/1 relation 
			rg.lChar = (*sIt).at ( i ).unicode();
			rg.xadvance =  glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
			rg.yadvance =  glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
			rg.xoffset = 0;
			rg.yoffset = 0;
			delete glyph;
			if(hl.contains( i ))
			{
// 				qDebug()<<"H B A"<<i<<hl[i].first<<hl[i].second;
				rg.isBreak = true;
				QString addOnFirst;
				QString addOnSecond;
				addOnFirst =  hl[i].first.endsWith("-") ? "": "-";
				addOnSecond = /*(*sIt).endsWith(".")?".":*/"";
				QString bS(hl[i].first + addOnFirst);
				for(int bI(0);bI<bS.count();++bI)
				{
// 					qDebug()<<"i bI a"<<i<<bI<<bS.at ( bI );
					glyph = itemFromChar ( bS.at ( bI ).unicode(), fsize );
					if ( !glyph )
					{
						continue;
					}
					RenderedGlyph bg;
					bg.glyph = glyph->data(GLYPH_DATA_GLYPH).toInt();
// 					bg.log = ; // We are in a 1/1 relation 
					bg.lChar = bS.at ( bI ).unicode();
					bg.xadvance =  glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
					bg.yadvance =  glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
					bg.xoffset = 0;
					bg.yoffset = 0;
					delete glyph;
					rg.hyphen.first << bg;
					
				}
				bS = hl[i].second + addOnSecond ;
				for(int bI(0);bI<bS.count();++bI)
				{
					glyph = itemFromChar ( bS.at ( bI ).unicode(), fsize );
					if ( !glyph )
					{
						continue;
					}
					RenderedGlyph bg;
					bg.glyph = glyph->data(GLYPH_DATA_GLYPH).toInt();
// 					bg.log = i; // We are in a 1/1 relation 
					bg.lChar = bS.at ( bI ).unicode();
					bg.xadvance =  glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
					bg.yadvance =  glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
					bg.xoffset = 0;
					bg.yoffset = 0;
					delete glyph;
					rg.hyphen.second << bg;
					
				}
				
			}
			ret << rg;
		}
	}

	for(int s(0); s < endSpaceCount; ++s)
	{
		ret << wSpace;
	}
	releaseFace();
// 	qDebug()<<"EndOfGlyphs";
	return ret;
}

GlyphList FontItem::glyphs(QString spec, double fsize, OTFSet set)
{
	FMHyphenator *hyph = typotek::getInstance()->getHyphenator();
	GlyphList Gret;
	if ( spec.isEmpty() || fsize <= 0.0 || !m_isOpenType) // enough :-)
		return Gret;
	if(!ensureFace())
		return Gret;
	
	otf = new FMOtf ( m_face, 0x10000 );
	if ( !otf )
	{
		releaseFace();
		return Gret;
	}

//	FMAltContext * actx ( FMAltContextLib::GetCurrentContext());
//	int cword(0);
//	int cchunk(0);
	QStringList stl(spec.split(' ',QString::SkipEmptyParts));
	
	double scalefactor = fsize / m_face->units_per_EM  ;
	
	QGraphicsPathItem *glyph = itemFromChar ( QChar(' ').unicode() , fsize );
	RenderedGlyph wSpace(glyph->data(GLYPH_DATA_GLYPH).toInt(),0, glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor ,0,0,0,' ',false);
	wSpace.lChar = 0x20;
	delete glyph;
	for(QStringList::const_iterator sIt(stl.constBegin());sIt != stl.constEnd(); ++ sIt)
	{
//		actx->setWord(cword);
//		actx->setChunk(cchunk);
//		actx->fileWord(*sIt);
//		actx->fileChunk(*sIt);
		if(sIt != stl.constBegin())
		{
			Gret << wSpace;
		}
		HyphList hl;
		if(hyph)
		{
			hl = hyph->hyphenate(*sIt) ;
// 			qDebug()<<"Hyph W C"<<*sIt<<hl.count();
		}
		GlyphList ret( otf->procstring ( *sIt , set ) );
		// otf->procstring works in font unit, so...
		for(int i(0); i < ret.count(); ++i)
		{
			ret[i].xadvance *= scalefactor;
			ret[i].yadvance *= scalefactor;
			ret[i].xoffset *= scalefactor;
			ret[i].yoffset *= scalefactor;
			
			if(hl.contains( ret[i].log ))
			{
// 				qDebug()<<"L R"<<hl[ret[i].log].first<<hl[ret[i].log].second;
				ret[i].isBreak = true;
				QString addOnFirst;
				QString addOnSecond;
				addOnFirst =  hl[i].first.endsWith("-") ? "": "-";
// 				addOnSecond = (*sIt).endsWith(".")?".":"";
//				actx->setChunk(++cchunk);
//				actx->fileChunk( hl[ret[i].log].first + addOnFirst);
				ret[i].hyphen.first = otf->procstring ( hl[ret[i].log].first + addOnFirst, set );
				for(int f(0); f < ret[i].hyphen.first.count(); ++f)
				{
					ret[i].hyphen.first[f].xadvance *= scalefactor;
					ret[i].hyphen.first[f].yadvance *= scalefactor;
					ret[i].hyphen.first[f].xoffset *= scalefactor;
					ret[i].hyphen.first[f].yoffset *= scalefactor;
				}

//				actx->setChunk(++cchunk);
//				actx->fileChunk( hl[ret[i].log].second + addOnSecond );
				ret[i].hyphen.second = otf->procstring ( hl[ret[i].log].second + addOnSecond, set );
				for(int f(0); f < ret[i].hyphen.second.count(); ++f)
				{
					ret[i].hyphen.second[f].xadvance *= scalefactor;
					ret[i].hyphen.second[f].yadvance *= scalefactor;
					ret[i].hyphen.second[f].xoffset *= scalefactor;
					ret[i].hyphen.second[f].yoffset *= scalefactor;
				}
			}
		}
		
		Gret << ret;
//		cchunk = 0;
//		++cword;
		
	}
	delete otf;
	otf = 0;
	releaseFace();
	return Gret;
}

GlyphList FontItem::glyphs(QString spec, double fsize, QString script)
{
	FMHyphenator *hyph = typotek::getInstance()->getHyphenator();
	GlyphList Gret;
	if ( spec.isEmpty() || fsize <= 0.0 || !m_isOpenType) // enough :-)
		return Gret;
	if(!ensureFace())
		return Gret;

	otf = new FMOtf ( m_face, 0x10000 );
	if ( !otf )
	{
		releaseFace();
		return Gret;
	}
	FMShaperFactory *shaperfactory = 0;
//	switch(m_shaperType)
//	{
//		case FMShaperFactory::FONTMATRIX : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
//		break;
//		case FMShaperFactory::HARFBUZZ : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::HARFBUZZ );
//		break;
//		case FMShaperFactory::ICU : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::ICU );
//		break;
//		case FMShaperFactory::M17N : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::M17N );
//		break;
//		case FMShaperFactory::PANGO : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::PANGO );
//		break;
//		case FMShaperFactory::OMEGA : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::OMEGA);
//		break;
//		default : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
//	}
	shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::ICU );
	
	
	/// HYPHENATION 
	
	QStringList stl(spec.split(' ',QString::SkipEmptyParts));
	
	double scalefactor = fsize / m_face->units_per_EM  ;
	QGraphicsPathItem *glyph = itemFromChar ( QChar(' ').unicode() , fsize );
	RenderedGlyph wSpace(glyph->data(GLYPH_DATA_GLYPH).toInt(),0, glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor ,0,0,0,' ',false);
	wSpace.lChar = 0x20;
	delete glyph;
	
	QMap<QString, GlyphList> cache;
	for(QStringList::const_iterator sIt(stl.constBegin());sIt != stl.constEnd(); ++ sIt)
	{
		if(cache.contains(*sIt))
		{
			GlyphList ret( cache.value(*sIt) );
			Gret <<  wSpace << ret;
			continue;
		}
		if(sIt != stl.constBegin())
		{
			Gret << wSpace;
		}
		HyphList hl;
		if(hyph)
		{
			hl = hyph->hyphenate(*sIt) ;
		}
		
		GlyphList ret( shaperfactory->doShape( *sIt ) );
		
		for(int i(0); i < ret.count(); ++i)
		{
			ret[i].xadvance *= scalefactor;
			ret[i].yadvance *= scalefactor;
			ret[i].xoffset *= scalefactor;
			ret[i].yoffset *= scalefactor;
			
			if(hl.contains( ret[i].log ))
			{
				ret[i].isBreak = true;
				QString addOnFirst;
				QString addOnSecond;
				addOnFirst =  hl[i].first.endsWith("-") ? "": "-";
				
				ret[i].hyphen.first = shaperfactory->doShape ( hl[ret[i].log].first + addOnFirst );
				for(int f(0); f < ret[i].hyphen.first.count(); ++f)
				{
					ret[i].hyphen.first[f].xadvance *= scalefactor;
					ret[i].hyphen.first[f].yadvance *= scalefactor;
					ret[i].hyphen.first[f].xoffset *= scalefactor;
					ret[i].hyphen.first[f].yoffset *= scalefactor;
				}
				
				ret[i].hyphen.second = shaperfactory->doShape ( hl[ret[i].log].second + addOnSecond );
				for(int f(0); f < ret[i].hyphen.second.count(); ++f)
				{
					ret[i].hyphen.second[f].xadvance *= scalefactor;
					ret[i].hyphen.second[f].yadvance *= scalefactor;
					ret[i].hyphen.second[f].xoffset *= scalefactor;
					ret[i].hyphen.second[f].yoffset *= scalefactor;
				}
			}
		}
		
		Gret << ret;
		cache[*sIt] = ret;
		
		
	}
	
	/// END OF HYPHENATION
	
	delete shaperfactory;
	delete otf;
	otf = 0;
	releaseFace();
// 	foreach(RenderedGlyph g, Gret)
// 	{
// 		g.dump();
// 	}
	return Gret;
}


double FontItem::getUnitPerEm()
{
	if(unitPerEm)
		return unitPerEm;
	if(ensureFace())
	{
		releaseFace();
	}
	return unitPerEm;
}



unsigned int FontItem::getFTHintMode() const
{
	return m_FTHintMode;
}


void FontItem::setFTHintMode ( unsigned int theValue )
{
	m_FTHintMode = theValue;
}

// here for migration purpose
void FontItem::dumpIntoDB()
{
	if ( !m_valid )
		return;

	FMFontDb *db ( FMFontDb::DB() );
	db->initRecord ( m_path );

	QString panString ( panose() );

	QList<FMFontDb::Field> fl;
	QVariantList vl;
	
	fl << FMFontDb::Family
	<< FMFontDb::Variant
	<< FMFontDb::Name
	<< FMFontDb::Type
	<< FMFontDb::Panose;
	
	vl << m_family
	<< m_variant
	<< m_name
	<< m_type
	<< panString;

	db->setValues ( m_path,fl,vl );
	db->setInfoMap ( m_path, moreInfo() );
}

QStringList FontItem::charmaps()
{
	QStringList ret;
	foreach(FT_Encoding e, m_charsets)
	{
		ret << FontStrings::Encoding(e);
	}
	return ret;
}

QString FontItem::renderSVG(const QString & s, const double& size)
{
	if(!ensureFace())
		return QString();
	
	QString ret;
	QString svg;
	QTransform tf;
	double pifs ( size );
	double scaleFactor( pifs / unitPerEm );
	double vertOffset ( pifs );
	double horOffset ( 0 );
	tf.translate ( horOffset , vertOffset );

	foreach (const QChar& c, s )
	{
		{
			QGraphicsPathItem * gpi ( itemFromChar ( c.unicode(), pifs ) );
			if ( gpi )
			{
				GlyphToSVGHelper gtsh ( gpi->path(), tf );
				svg += gtsh.getSVGPath();
				horOffset += gpi->data(GLYPH_DATA_HADVANCE).toDouble() * scaleFactor;
				tf.translate( gpi->data(GLYPH_DATA_HADVANCE).toDouble()  * scaleFactor,0 );
				delete gpi;
			}
		}
	}
	QString openElem ( QString ( "<svg width=\"%1\" height=\"%2\"  xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" )
			.arg ( horOffset )
			.arg ( m_face->height * scaleFactor ) );
	ret += openElem;
	ret += svg;
	ret += "</svg>";
	
	releaseFace();
	return ret;
}

unsigned short FontItem::getNamedChar(const QString & name)
{
	if(!ensureFace())
		return 0;
	unsigned short ret(0);
	
	if(FT_HAS_GLYPH_NAMES( m_face ))
	{
		int bLen(256);
		char* buffer(new char[bLen]);
		FT_UInt index (1);
		FT_UInt cc =  FT_Get_First_Char ( m_face, &index );
		QString cname;
		while ( index )
		{
			FT_Get_Glyph_Name(m_face, index , buffer, bLen);
			cname = QString::fromLatin1(buffer);
// 			qDebug()<<"NC"<<cname<<cc<<index;
			if(0 == name.compare(cname))
			{
					ret = cc;
					break;
			}
			cc = FT_Get_Next_Char ( m_face, cc, &index );
		}
		delete[] buffer;
	}
	
	releaseFace();
	return ret;
}

QStringList FontItem::getNames()
{
	QStringList ret;
	if(!ensureFace())
		return ret;
	if(FT_HAS_GLYPH_NAMES( m_face ))
	{
		int bLen(256);
		char* buffer(new char[bLen]);
		FT_UInt index (1);
		FT_UInt cc =  FT_Get_First_Char ( m_face, &index );
		QString cname;
		while ( index )
		{
			FT_Get_Glyph_Name(m_face, index , buffer, bLen);
			ret << QString::fromLatin1(buffer);
			cc = FT_Get_Next_Char ( m_face, cc, &index );
		}
		delete[] buffer;
	}
	releaseFace();
	return ret;
}

void FontItem::exploreKernFeature()
{
	if(!ensureFace())
		return;
	
	FMKernFeature kf(m_face);
	
	releaseFace();
}

bool FontItem::getUnicodeBuiltIn() const
{
	return m_unicodeBuiltIn;
}


FT_Encoding FontItem::getCurrentEncoding() const
{
	return m_currentEncoding;
}


double FontItem::getUnitPerEm() const
{
	return unitPerEm;
}


QList< FT_Encoding > FontItem::getCharsets() const
{
	return m_charsets;
}
