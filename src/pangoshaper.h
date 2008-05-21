//
// C++ Interface: pangoshaper
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PANGOSHAPER_H
#define PANGOSHAPER_H

#include "fmbaseshaper.h"

class PangoShaper : public FMBaseShaper
{
	public:
		PangoShaper(FMOtf* o, QString s);
		~PangoShaper();
		GlyphList doShape( const QString& s );
};

#endif

