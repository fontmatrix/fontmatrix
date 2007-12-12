//
// C++ Implementation: fmshaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fmshaper.h"

#include <QList>
#include <QMap>
#include <QDebug>


HB_GraphemeClass HB_GetGraphemeClass(HB_UChar32 ch)
{
	return (HB_GraphemeClass) 0;
}
HB_WordClass HB_GetWordClass(HB_UChar32 ch)
{
	return (HB_WordClass) 0;
}
HB_SentenceClass HB_GetSentenceClass(HB_UChar32 ch)
{
	return (HB_SentenceClass) 0;
}

void HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *grapheme, HB_LineBreakClass *lineBreak)
{
	//###
}

static HB_Error hb_getSFntTable(void *font, HB_Tag tableTag, HB_Byte *buffer, HB_UInt *length)
{
	FT_Face face = (FT_Face)font;
	FT_ULong ftlen = *length;
	FT_Error error = 0;

	if (!FT_IS_SFNT(face))
		return HB_Err_Invalid_Argument;

	error = FT_Load_Sfnt_Table(face, tableTag, 0, buffer, &ftlen);
	*length = ftlen;
	return (HB_Error)error;
}

/***************** UTILS ************/
namespace 
{	
	HB_Script script2script ( QString script )
	{
		QMap<QString, HB_Script> hbscmap;
		hbscmap["arab"] = HB_Script_Arabic ;
		hbscmap["armn"] = HB_Script_Armenian ;
		hbscmap["beng"] = HB_Script_Bengali ;
		hbscmap["cyrl"] = HB_Script_Cyrillic ;
		hbscmap["deva"] = HB_Script_Devanagari ;
		hbscmap["geor"] = HB_Script_Georgian ;
		hbscmap["grek"] = HB_Script_Greek ;
		hbscmap["gujr"] = HB_Script_Gujarati ;
		hbscmap["guru"] = HB_Script_Gurmukhi ;
		hbscmap["hang"] = HB_Script_Hangul ;
		hbscmap["hebr"] = HB_Script_Hebrew ;
		hbscmap["knda"] = HB_Script_Kannada ;
		hbscmap["khmr"] = HB_Script_Khmer ;
		hbscmap["lao "] = HB_Script_Lao ;
		hbscmap["mlym"] = HB_Script_Malayalam ;
		hbscmap["mymr"] = HB_Script_Myanmar ;
		hbscmap["ogam"] = HB_Script_Ogham ;
		hbscmap["orya"] = HB_Script_Oriya ;
		hbscmap["runr"] = HB_Script_Runic ;
		hbscmap["sinh"] = HB_Script_Sinhala ;
		hbscmap["syrc"] = HB_Script_Syriac ;
		hbscmap["taml"] = HB_Script_Tamil ;
		hbscmap["telu"] = HB_Script_Telugu ;
		hbscmap["thaa"] = HB_Script_Thaana ;
		hbscmap["thai"] = HB_Script_Thai ;
		hbscmap["tibt"] = HB_Script_Tibetan ;
	
		HB_Script ret = hbscmap.contains ( script ) ? hbscmap[script] : HB_Script_Common;
		return   ret;
	}

}

FmShaper::FmShaper()
{
	faceisset = langisset = allocated = false;
}

FmShaper::~ FmShaper()
{
	if (faceisset)
	{
		delete m.font;
		HB_FreeFace(m.face);
	}
}

bool FmShaper::setFont (FT_Face face, HB_Font font  )
{
	HB_Face hbFace = HB_NewFace(face, hb_getSFntTable);
	HB_Font hbFont = new HB_FontRec;
	
	hbFont->klass = font->klass ;
	hbFont->userData = font->userData;
	hbFont->x_ppem  = font->x_ppem;
	hbFont->y_ppem  = font->y_ppem;
	hbFont->x_scale = font->x_scale;
	hbFont->y_scale = font->y_scale;
	m.font = hbFont;
	m.face = hbFace;
	faceisset = true;
	return faceisset;
}


bool FmShaper::setScript ( QString script )
{
	HB_Script ret = script2script ( script );
	if ( ret != HB_Script_Common )
	{
		m.item.script = ret;
		return true;
	}
	return false;
}

// Actual work begins here :-)

int FmShaper::doShape ( QString str )
{

	m.kerning_applied = false;
	m.string = reinterpret_cast<const HB_UChar16 *> ( str.constData() );

	m.stringLength = str.length();
	m.item.pos = 0;
	m.item.bidiLevel = 0;
	m.shaperFlags = HB_ShaperFlag_UseDesignMetrics;
	m.num_glyphs = m.item.length = m.stringLength;
	m.glyphIndicesPresent = false;
	m.initialGlyphCount = 0;

	if ( allocated )
	{
		delete  m.glyphs;
		delete  m.attributes;
		delete  m.advances;
		delete  m.offsets;
		delete  m.log_clusters;
	}
	int neededspace = m.num_glyphs  ;


	m.glyphs = new HB_Glyph[neededspace];
	memset ( m.glyphs, 0, neededspace * sizeof ( HB_Glyph ) );
	m.attributes = new HB_GlyphAttributes[neededspace];
	memset ( m.attributes, 0, neededspace * sizeof ( HB_GlyphAttributes ) );
	m.advances = new HB_Fixed[neededspace];
	memset ( m.advances, 0, neededspace * sizeof ( HB_Fixed ) );
	m.offsets = new HB_FixedPoint[neededspace];
	memset ( m.offsets, 0, neededspace * sizeof ( HB_FixedPoint ) );
	m.log_clusters = new unsigned short[neededspace];

	allocated = true;

//	qDebug(QString("goto HB_ShapeItem, num_glyphs = %1").arg(m.num_glyphs));
	HB_Bool result = HB_ShapeItem ( &m );
//	qDebug(QString("come back from HB_ShapeItem, num_glyphs = %1, %2").arg(m.num_glyphs).arg(result ? "TRUE" :"FALSE"));
	if ( !result )
	{
		neededspace = m.num_glyphs  ;

		delete m.glyphs;
		delete m.attributes;
		delete m.advances;
		delete m.offsets;
		delete m.log_clusters;

		m.glyphs = new HB_Glyph[neededspace];
		memset ( m.glyphs, 0, neededspace * sizeof ( HB_Glyph ) );
		m.attributes = new HB_GlyphAttributes[neededspace];
		memset ( m.attributes, 0, neededspace * sizeof ( HB_GlyphAttributes ) );
		m.advances = new HB_Fixed[neededspace];
		memset ( m.advances, 0, neededspace * sizeof ( HB_Fixed ) );
		m.offsets = new HB_FixedPoint[neededspace];
		memset ( m.offsets, 0, neededspace * sizeof ( HB_FixedPoint ) );
		m.log_clusters = new unsigned short[neededspace];

		result = HB_ShapeItem ( &m );
		//qDebug(QString("come back from HB_ShapeItem, num_glyphs = %1, %2").arg(m.num_glyphs).arg(result ? "TRUE" :"FALSE"));
	}
	return m.stringLength;

}


HB_Buffer  FmShaper::out_buffer()
{
	return m.face->buffer;
}




