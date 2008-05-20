//
// C++ Implementation: m17nshaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "m17nshaper.h"

M17NShaper * M17NShaper::instance = 0;
		
QString OTF_tag_name ( unsigned int tag );

M17NShaper::M17NShaper ( FMOtf * o, QString s )
		:FMBaseShaper ( o,s )
{
	instance = this;
	M17N_INIT();
			
	mFont.family = Mnil /*otf->face()->family_name*/;
	mFont.x_ppem = otf->face()->units_per_EM;
	mFont.y_ppem = otf->face()->units_per_EM;
	mFont.get_glyph_id = impl_get_glyph_id;
	mFont.get_metrics = impl_get_metrics ;
	mFont.check_otf = impl_check_otf;
	mFont.drive_otf = impl_drive_otf;
	
	
}

M17NShaper::~ M17NShaper()
{
	M17N_FINI() ;
}

GlyphList M17NShaper::doShape ( const QString & s )
{
	GlyphList ret;
	if(s.isEmpty())
		return ret;
	
	grrr = mflt_find (s.at(0).unicode(), &mFont);
	MFLTGlyphString * gs = new MFLTGlyphString;
	MFLTGlyph * gl = new MFLTGlyph[s.count()];
	gs->glyph_size = sizeof (MFLTGlyph) * (s.count() );
	gs->glyphs = gl;
	gs->allocated = s.count() ;
	for(int i(0); i < s.count(); ++i)
	{
		gl[i].c = s[i].unicode();
	}
	
	mflt_run(gs, 0, s.count() , &mFont, grrr);
}


/// One again that you can’t just attach a font file or face to :(
int M17NShaper::impl_get_glyph_id( struct _MFLTFont *font, MFLTGlyphString *gstring, int from, int to )
{
	FT_Face face = instance->otf->face();
	if(!face)
		return 8; // corresponds to "#define OTF_ERROR_FT_FACE 8" in libotf 
	for(int idx(from); idx < to; ++idx)
	{
		gstring->glyphs[idx].code = FT_Get_Char_Index(face, gstring->glyphs[idx].c);
		gstring->glyphs[idx].encoded = 1;
	}
	return 0;	
}
int M17NShaper::impl_get_metrics( struct _MFLTFont *font, MFLTGlyphString *gstring, int from, int to )
{
	// I wonder if it will be called, normally no...
	FT_Face face = instance->otf->face();
	if(!face)
		return 8; // corresponds to "#define OTF_ERROR_FT_FACE 8" in libotf 
	for(int idx(from); idx < to; ++idx)
	{
		if(FT_Load_Glyph(face, gstring->glyphs[idx].code,  FT_LOAD_NO_SCALE ))
			continue;
		//TODO convert values to 26.6
		gstring->glyphs[idx].xadv = face->glyph->metrics.horiAdvance ;
		gstring->glyphs[idx].yadv = face->glyph->metrics.vertAdvance ;
		gstring->glyphs[idx].ascent = face->ascender ;
		gstring->glyphs[idx].descent= face->descender ;
		gstring->glyphs[idx].lbearing= face->glyph->metrics.horiBearingX ;
		gstring->glyphs[idx].rbearing= face->glyph->metrics.horiBearingX + face->glyph->metrics.width ;
		gstring->glyphs[idx].measured = 1;
	}
	return 0;
}
int M17NShaper::impl_check_otf( struct _MFLTFont *font, MFLTOtfSpec *spec )
{
	return 1; // programming is easy ;-)
}
int M17NShaper::impl_drive_otf ( struct _MFLTFont *font, MFLTOtfSpec *spec, MFLTGlyphString *in, int from, int to, MFLTGlyphString *out, MFLTGlyphAdjustment *adjustment )
{
	instance->cachedString.clear();
	QString script ( OTF_tag_name ( spec->script ) );
	QString lang ( "dflt" );
	QStringList subf;
	QStringList posf;
	QList<unsigned int> gl;
	unsigned int* cursor ( 0 );
	for ( cursor = spec->features[0]; *cursor ; ++cursor )
	{
		subf <<  OTF_tag_name ( *cursor );
	}
	for ( cursor = spec->features[1]; *cursor ; ++cursor )
	{
		posf <<  OTF_tag_name ( *cursor );
	}
	for(int i(from); i < to; ++i)
	{
		gl << in->glyphs[i].code;
	}
	
	instance->cachedString = instance->otf->procstring(gl,script,lang,subf,posf);
}








