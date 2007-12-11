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

/***************** UTILS ************/
HB_Script script2script ( QString script )
{
	static QMap<QString, HB_Script> hbscmap;
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
	if ( hbscmap.keys().contains ( script ) )
		return hbscmap[script];
	return HB_Script_Common;
}




// static HB_UChar32 getChar ( const HB_UChar16 *string, hb_uint32 length, hb_uint32 &i )
// {
// 	HB_UChar32 ch;
// 	if ( HB_IsHighSurrogate ( string[i] )
// 	        && i < length - 1
// 	        && HB_IsLowSurrogate ( string[i + 1] ) )
// 	{
// 		ch = HB_SurrogateToUcs4 ( string[i], string[i + 1] );
// 		++i;
// 	}
// 	else
// 	{
// 		ch = string[i];
// 	}
// 	return ch;
// }
// 
// static HB_Bool hb_stringToGlyphs ( HB_Font font, const HB_UChar16 *string, hb_uint32 length, HB_Glyph *glyphs, hb_uint32 *numGlyphs, HB_Bool /*rightToLeft*/ )
// {
// 	FT_Face face = ( FT_Face ) font->userData;
// 	if ( length > *numGlyphs )
// 		return false;
// 
// 	int glyph_pos = 0;
// 	for ( hb_uint32 i = 0; i < length; ++i )
// 	{
// 		glyphs[glyph_pos] = FT_Get_Char_Index ( face, getChar ( string, length, i ) );
// 		++glyph_pos;
// 	}
// 
// 	*numGlyphs = glyph_pos;
// 
// 	return true;
// }
// 
// static void hb_getAdvances ( HB_Font font, const HB_Glyph * glyphs, hb_uint32 numGlyphs, HB_Fixed *advances, int flags )
// {
// 	FT_Face face = ( FT_Face ) font->userData;
// 	FT_Glyph_Metrics m;
// 	for ( hb_uint32 i = 0; i < numGlyphs; ++i )
// 	{
// 
// 		if ( !FT_Load_Glyph ( face, glyphs[i], FT_LOAD_NO_SCALE ) )
// 		{
// 			m = face->glyph->metrics;
// 			advances[i] = m.horiAdvance; // ### not tested right now
// 		}
// 		else
// 		{
// // 			qDebug ( QString ( "unable to load glyph %1" ).arg ( glyphs[i] ) );
// 			advances[i] = 0;
// 		}
// 	}
// }
// 
// static HB_Bool hb_canRender ( HB_Font font, const HB_UChar16 *string, hb_uint32 length )
// {
// 	FT_Face face = ( FT_Face ) font->userData;
// 
// 	for ( hb_uint32 i = 0; i < length; ++i )
// 		if ( !FT_Get_Char_Index ( face, getChar ( string, length, i ) ) )
// 			return false;
// 
// 	return true;
// }
// 
// static HB_Error hb_getSFntTable ( void *font, HB_Tag tableTag, HB_Byte *buffer, HB_UInt *length )
// {
// 	FT_Face face = ( FT_Face ) font;
// 	FT_ULong ftlen = *length;
// 	FT_Error error = 0;
// 
// 	if ( !FT_IS_SFNT ( face ) )
// 		return HB_Err_Invalid_Argument;
// 
// 	error = FT_Load_Sfnt_Table ( face, tableTag, 0, buffer, &ftlen );
// 	if ( ftlen > 32 )
// 		*length = ftlen;
// 	else
// 	{
// 		length = 0;
// 		return ( HB_Error ) HB_Err_Invalid_SubTable;
// 	}
// 	return ( HB_Error ) error;
// }
// 
// HB_Error hb_getPointInOutline ( HB_Font font, HB_Glyph glyph, int flags, hb_uint32 point, HB_Fixed *xpos, HB_Fixed *ypos, hb_uint32 *nPoints )
// {
// 	HB_Error error = HB_Err_Ok;
// 	FT_Face face = ( FT_Face ) font->userData;
// 
// 	int load_flags = ( flags & HB_ShaperFlag_UseDesignMetrics ) ? FT_LOAD_NO_HINTING : FT_LOAD_DEFAULT;
// 
// 	if ( ( error = ( HB_Error ) FT_Load_Glyph ( face, glyph, load_flags ) ) )
// 		return error;
// 
// 	if ( face->glyph->format != ft_glyph_format_outline )
// 		return ( HB_Error )HB_Err_Invalid_SubTable ;
// 
// 	*nPoints = face->glyph->outline.n_points;
// 	if ( ! ( *nPoints ) )
// 		return HB_Err_Ok;
// 
// 	if ( point > *nPoints )
// 		return ( HB_Error ) HB_Err_Invalid_SubTable;
// 
// 	*xpos = face->glyph->outline.points[point].x;
// 	*ypos = face->glyph->outline.points[point].y;
// 
// 	return HB_Err_Ok;
// }
// 
// void hb_getGlyphMetrics ( HB_Font font, HB_Glyph glyph, HB_GlyphMetrics *metrics )
// {
// 	// ###
// 	qDebug ( "hb_getGlyphMetrics" );
// 	metrics->x = metrics->y = metrics->width = metrics->height = metrics->xOffset = metrics->yOffset = 0;
// 	FT_Face _face = ( FT_Face ) font;
// 
// 	if ( !FT_Load_Glyph ( _face, glyph, FT_LOAD_NO_SCALE ) )
// 	{
// 		FT_Glyph_Metrics m = _face->glyph->metrics;
// 		metrics->x = m.horiAdvance;
// 		metrics->y = m.vertAdvance;
// 		metrics->width = m.width;
// 		metrics->height = m.height;
// 		metrics->xOffset = m.horiBearingX;
// 		metrics->yOffset = m.horiBearingY;
// 	}
// 
// 
// }
// 
// HB_Fixed hb_getFontMetric ( HB_Font font, HB_FontMetric metric )
// {
// 	FT_Face _face = ( FT_Face ) font;
// 	return _face->height; // ####
// }

/***********************************/


// FmShaper::FmShaper(FT_Face ftface, QString lang)
// {
// 	faceisset = langisset = false;
// 	setFace(ftface);
// 	setLang(lang);
// }
//
//
// FmShaper::FmShaper(FT_Face ftface)
// {
// 	faceisset = langisset = false;
// 	setFace(ftface);
// }

FmShaper::FmShaper()
{
	faceisset = langisset = allocated = false;
}

FmShaper::~ FmShaper()
{
}
bool FmShaper::setFont (HB_Font hbfont  )
{
	
	m.font = &hbFont;
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




