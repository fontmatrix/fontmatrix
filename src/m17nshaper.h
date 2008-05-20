//
// C++ Interface: m17nshaper
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef M17NSHAPER_H
#define M17NSHAPER_H

#include "fmbaseshaper.h"
#include <m17n-flt.h>


class M17NShaper : public FMBaseShaper
{
	public:
		M17NShaper(FMOtf* o, QString s);
		~M17NShaper();
	
		GlyphList doShape( const QString& s );
		
		
		static int impl_get_glyph_id( struct _MFLTFont *font, MFLTGlyphString *gstring, int from, int to );
		static int impl_get_metrics( struct _MFLTFont *font, MFLTGlyphString *gstring, int from, int to );
		static int impl_check_otf( struct _MFLTFont *font, MFLTOtfSpec *spec );
		static int impl_drive_otf ( struct _MFLTFont *font, MFLTOtfSpec *spec, MFLTGlyphString *in, int from, int to, MFLTGlyphString *out, MFLTGlyphAdjustment *adjustment );
		
		
	private:
		// I hate callback functions
		static M17NShaper *instance;
	public:
		GlyphList cachedString;
		MFLTFont mFont;
		MFLT *grrr;
		
};

#endif

