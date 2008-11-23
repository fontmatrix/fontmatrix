//
// C++ Implementation: pangoshaper
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "pangoshaper.h"

PangoShaper::PangoShaper(FMOtf * o, QString s)
	:FMBaseShaper(o,s)
{
}

PangoShaper::~ PangoShaper()
{
}

GlyphList PangoShaper::doShape(const QString & s)
{
	return GlyphList();
}



