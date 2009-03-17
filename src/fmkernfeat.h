//
// C++ Interface: fmkernfeat
//
// Description: Test how to build a kern table out of a kern feature
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMKERNFEAT_H
#define FMKERNFEAT_H

#include <QByteArray>
#include <QList>
#include <QMap>

#include <ft2build.h>
#include FT_FREETYPE_H

class FMKernFeature
{
	public:
		FMKernFeature(FT_Face face);
		~FMKernFeature();
		
		
	private:
		QByteArray GPOSTableRaw;
		QList<unsigned int> coverage;
		QMap<unsigned int, unsigned int> pairs;
		
		void makeCoverage();
		void makePairs();
		
		// return a uint16 from position index in GPOSTableRaw
		inline quint16 toUint16(int index);
};


#endif // FMKERNFEAT_H


