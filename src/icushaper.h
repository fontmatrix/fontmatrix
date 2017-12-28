//
// C++ Interface: icushaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ICUSHAPER_H
#define ICUSHAPER_H

#include "fmbaseshaper.h"

#include <layout/LETypes.h>
#include <layout/LayoutEngine.h>
#include <layout/LEFontInstance.h>
#include <layout/LEScripts.h>

class IcuFontImpl : public LEFontInstance
{
	public:
		IcuFontImpl ( FMOtf* o );
		~IcuFontImpl();
		// implements pure virtual methods of LEFontInstance

		const void* getFontTable(LETag   tableTag, size_t &length_sz ) const;
		le_bool 	canDisplay (LEUnicode32 ch) const {return true;}
		le_int32 	getUnitsPerEM () const;
		LEGlyphID 	mapCharToGlyph (LEUnicode32 ch) const ;
		void 	getGlyphAdvance (LEGlyphID glyph, LEPoint &advance) const;
		le_bool 	getGlyphPoint (LEGlyphID glyph, le_int32 pointNumber, LEPoint &point) const;
		float 	getXPixelsPerEm () const ;
		float 	getYPixelsPerEm () const ;
		float 	getScaleFactorX () const ;
		float 	getScaleFactorY () const ;
		le_int32 	getAscent () const ;
		le_int32 	getDescent () const;
		le_int32 	getLeading () const;
		
		static void regTables(LETag   tableTag, unsigned char *t){instance->tables[tableTag] = t;}
		

	private:
		FMOtf *otf;
		static IcuFontImpl *instance;
		QMap<LETag,unsigned char*> tables;
};


class IcuShaper : public FMBaseShaper
{
	public:
		IcuShaper ( FMOtf* o, QString s );
		~IcuShaper();

		GlyphList doShape ( const QString& s );

	private:
		LayoutEngine *icuLE;
		IcuFontImpl *icuFont;
		
		mutable QMap<QString, unsigned int> tagToCode;
		void fillTagToCode();
		void IcuError(int err);
};



#endif

