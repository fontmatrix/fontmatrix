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
#include "harfbuzzqtshaper.h"

#include <QDebug>



QStringList FMShaperFactory::types()
{
	QStringList ret;
	ret.clear();
	ret << "FONTMATRIX";
	ret << "HARFBUZZ";
	ret << "ICU";
// 	ret << "M17N";
// 	ret << "PANGO";
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
			case HARFBUZZ: 
				qDebug()<< "NEW HarfbuzzQtShaper";
				shaperImpl = new HarfbuzzQtShaper ( otf, script );
				break;
// 			case HARFBUZZ_PANGO: shaperImpl = new HarbuzzPangoShaper;
// 			break;
// 			case PANGO: shaperImpl = new PangoShaper ( otf, script );
// 				break;
// 			case ICU : shaperImpl = new IcuShaper ( otf, script );
// 				break;
// 			case M17N : shaperImpl = new M17NShaper ( otf, script );
// 				break;
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

