//
// C++ Implementation: harfbuzzqtshaper
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "harfbuzzqtshaper.h"


HarfbuzzShaper::HarfbuzzShaper(FMOtf * o, QString s)
	:FMBaseShaper(o,s)
{
	hbqtsh = new FMShaper(otf);
	hbqtsh->setScript ( script );
}

HarfbuzzShaper::~ HarfbuzzShaper()
{
	if(hbqtsh)
		delete hbqtsh;
}

GlyphList HarfbuzzShaper::doShape(const QString& s)
{
	return hbqtsh->doShape ( s , true );
}



