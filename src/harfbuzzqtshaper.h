//
// C++ Interface: harfbuzzqtshaper
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef HARFBUZZQTSHAPER_H
#define HARFBUZZQTSHAPER_H

#include "fmbaseshaper.h"
#include "fmshaper.h"

class HarfbuzzQtShaper : public FMBaseShaper
{
	public:
		HarfbuzzQtShaper(FMOtf* o, QString s);
		~HarfbuzzQtShaper();	
		
		GlyphList doShape( const QString& s );
	private:
		FMShaper *hbqtsh;
};

#endif

