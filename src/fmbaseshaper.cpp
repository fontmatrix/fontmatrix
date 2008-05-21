//
// C++ Implementation: fmbaseshaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "fmbaseshaper.h"

#include "fontmatrixshaper.h"

#ifdef HAVE_HARFBUZZ
#include "harfbuzzqtshaper.h"
#endif

#ifdef HAVE_ICU
#include "icushaper.h"
#endif

#ifdef HAVE_M17N
#include "m17nshaper.h"
#endif

#ifdef HAVE_PANGO
#include "pangoshaper.h"
#endif

#include <QDebug>



QMap<QString, int> FMShaperFactory::types()
{
	QMap<QString, int> ret;
	ret.clear();
	ret["Fontmatrix"] = FONTMATRIX;
	
#ifdef HAVE_HARFBUZZ
	ret["Harfbuzz"] = HARFBUZZ;
#endif
	
#ifdef HAVE_ICU
	ret["ICU"] = ICU;
#endif
	
#ifdef HAVE_M17N
	ret["m17n"] = M17N;
#endif
	
#ifdef HAVE_PANGO
	ret["Pango"] = PANGO;
#endif
// 	ret << "OMEGA";
	
	return ret;
	
	
}

FMShaperFactory::FMShaperFactory ( FMOtf * o, QString s, SHAPER_TYPE st )
		:otf ( o ), script ( s ), shaperType ( st )
{
	qDebug()<<"Creating Shaper FMShaperFactory"<< s ;
	shaperImpl = 0;
}

FMShaperFactory::~ FMShaperFactory()
{
	if ( shaperImpl )
		delete shaperImpl;
}


void FMShaperFactory::resetShaperType ( SHAPER_TYPE st )
{
	if ( shaperType == st )
		return;

	if ( shaperImpl )
	{
		delete shaperImpl;
		shaperImpl = 0;
	}

	shaperType = st;
}

GlyphList FMShaperFactory::doShape ( const QString & aString )
{
	if ( !shaperImpl )
	{
		switch ( shaperType )
		{
			case FONTMATRIX : 
				qDebug()<< "NEW FontmatrixShaper";
				shaperImpl = new FontmatrixShaper ( otf, script );
				break;	
#ifdef HAVE_HARFBUZZ
			case HARFBUZZ: 
				qDebug()<< "NEW HarfbuzzShaper";
				shaperImpl = new HarfbuzzShaper ( otf, script );
				break;	
#endif
#ifdef HAVE_PANGO
			case PANGO: 
				qDebug()<< "NEW PangoShaper";
				shaperImpl = new PangoShaper ( otf, script );
				break;	
#endif
#ifdef HAVE_ICU
			case ICU : 
				qDebug()<< "NEW IcuShaper";
				shaperImpl = new IcuShaper ( otf, script );
				break;	
#endif
#ifdef HAVE_M17N
			case M17N : 
				qDebug()<< "NEW M17NShaper";
				shaperImpl = new M17NShaper ( otf, script );
				break;
#endif
// 			case OMEGA : shaperImpl = new OmegaShaper ( otf, script );
// 				break;
			default:break;
		}
	}

	return shaperImpl->doShape ( aString );
}

FMBaseShaper::FMBaseShaper(FMOtf * o, QString s)
	:otf(o), script(s)
{
	
}

FMBaseShaper::~ FMBaseShaper()
{
}

