//
// C++ Interface: fmsharestruct
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMSHARESTRUCT_H
#define FMSHARESTRUCT_H

#include <QList>

// #include QString

struct RenderedGlyph
{
	int glyph;
	int log;
	double xadvance;
	double yadvance;
	double xoffset;
	double yoffset;
	unsigned short lChar;
};

typedef QList<RenderedGlyph> GlyphList;

#endif
