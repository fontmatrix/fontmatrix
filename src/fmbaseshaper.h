//
// C++ Interface: fmbaseshaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

/// "Don’t be rude dude, it’s my first strategy!" - Lao Tseu

#ifndef FMBASESHAPER_H
#define FMBASESHAPER_H

#include <QString>
#include <QStringList>
#include "fmotf.h"


/// There’s not too much to say, just subclass it and provide a method
/// which can deal with an OTF handle and a script to produce a glyphs string
/// A piece of cake :-)
class FMBaseShaper
{
	public:
		FMBaseShaper ( FMOtf *o, QString s );
		~FMBaseShaper();
		virtual GlyphList doShape ( const QString& aString ) = 0;
	protected:
		FMOtf *otf;
		QString script;
};


class FMShaperFactory
{
	public:
		enum SHAPER_TYPE{ FONTMATRIX = 1, // our dear own shaper
		                  HARFBUZZ,
		                  ICU,
		                  M17N,
		                  PANGO,
		                  OMEGA, // yes, I’ve red something about a C++ binding!
		                  NOT_A_SHAPER
	                };

		static QMap<QString, int> types();

		FMShaperFactory ( FMOtf *otf, QString script , SHAPER_TYPE st = FONTMATRIX );
		~FMShaperFactory();

		GlyphList doShape ( const QString& aString );
		void resetShaperType ( SHAPER_TYPE st = FONTMATRIX );

		// If you ever think to create your own "shaping strategy",
		// start by adding an entry here, half of the work :-)


	private:
		SHAPER_TYPE shaperType;
		FMOtf *otf;
		QString script;

		FMBaseShaper *shaperImpl;

};




#endif

