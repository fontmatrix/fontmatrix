//
// C++ Interface: fmshaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMSHAPER_H
#define FMSHAPER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H

#include <QString>
#include <QStringList>
#include "harfbuzz.h"
#include "fmotf.h" 

/**
	@author Pierre Marchand <pierremarc@oep-h.com>
*/
class FmShaper
{
	public:
		FmShaper(FmOtf *anchor);

		~FmShaper();
		/* Will return false if there is no GSUB nor GPOS table */
		bool setFont (/*FT_Face face, HB_Font font*/ );

		bool setScript ( QString script );

		
		QList<RenderedGlyph> doShape(QString string , bool ltr);

		HB_Buffer out_buffer();

	private:
		FmOtf *anchorOTF;
		FT_Face anchorFace;
		HB_ShaperItem m;

		bool faceisset;
		bool langisset;
		bool allocated;

		HB_FontRec hbFont;
		HB_FontClass fontClass;

};

#endif
