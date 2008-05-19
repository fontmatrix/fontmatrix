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
#include <m17n.h>

class M17NShaper : public FMBaseShaper
{
	public:
		M17NShaper(FMOtf* o, QString s);
		~M17NShaper();
	
		GlyphList doShape( const QString& s );
};

#endif

