//
// C++ Implementation: fontmatrixshaper
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fontmatrixshaper.h"

FontmatrixShaper::FontmatrixShaper(FMOtf * o, QString s)
	:FMBaseShaper(o,s)
{
	fmos = new FMOwnShaper(script);
}

FontmatrixShaper::~ FontmatrixShaper()
{
	if (fmos)
		delete fmos;
}

GlyphList FontmatrixShaper::doShape(const QString& s)
{
	fmos->fillIn(s);
	QList<Character> shaped ( fmos->GetShaped() );
	return otf->procstring( shaped, script );
}


