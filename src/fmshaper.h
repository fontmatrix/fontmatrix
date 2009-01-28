//
// C++ Interface: fmshaper
//
// Description: in fact represents the Harfbuzz shaper
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
#include "fmotf.h" 

/**
	@author Pierre Marchand <pierremarc@oep-h.com>
*/
class FMShaper
{
	public:
		FMShaper(FMOtf *anchor);

		~FMShaper();
		/* Will return false if there is no GSUB nor GPOS table */
		bool setFont (/*FT_Face face, HB_Font font*/ );

		bool setScript ( QString script );

		
		QList<RenderedGlyph> doShape(QString string , bool ltr);

		Harfbuzz::HB_Buffer out_buffer();

	private:
		FMOtf *anchorOTF;
		FT_Face anchorFace;
		Harfbuzz::HB_ShaperItem m;

		bool faceisset;
		bool langisset;
		bool allocated;

		Harfbuzz::HB_FontRec hbFont;
		Harfbuzz::HB_FontClass fontClass;

};

#endif
