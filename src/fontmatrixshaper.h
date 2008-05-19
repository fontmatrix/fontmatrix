//
// C++ Interface: fontmatrixshaper
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FONTMATRIXSHAPER_H
#define FONTMATRIXSHAPER_H

#include "fmbaseshaper.h"
#include "fmshaper_own.h"

class FontmatrixShaper : public FMBaseShaper
{
	public:
	FontmatrixShaper(FMOtf* o, QString s);
	~FontmatrixShaper();
	
	GlyphList doShape( const QString& s );
	private:
		FMOwnShaper *fmos;
};

#endif



