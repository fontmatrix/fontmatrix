//
// C++ Implementation: fmsharestruct
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmsharestruct.h"

RenderedGlyph::RenderedGlyph()
	:glyph(0),log(0),xadvance(0),yadvance(0),xoffset(0),yoffset(0),lChar(0),isBreak(false)
{
}

RenderedGlyph::RenderedGlyph(int g, int l, double xa, double ya, double xo, double yo, unsigned short c, bool b)
	:glyph(g),log(l),xadvance(xa),yadvance(ya),xoffset(xo),yoffset(yo),lChar(c),isBreak(b)
{
}

RenderedGlyph::~RenderedGlyph()
{
	foreach(RenderedGlyph* rg, hyphen.first)
	{
		if(rg)
			delete rg;
	}
	foreach(RenderedGlyph* rg, hyphen.second)
	{
		if(rg)
			delete rg;
	}
}

GlyphList RenderedGlyph::BeforeBreak()
{
	return hyphen.first;
}

GlyphList RenderedGlyph::AfterBreak()
{
	return hyphen.second;
}

void cleanupGlyphList(const GlyphList & list)
{
	foreach(RenderedGlyph* rg, list)
	{
		if(rg)
			delete rg;
	}
}




