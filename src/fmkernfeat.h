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
		FMKernFeature ( FT_Face face );
		~FMKernFeature();


	private:
		FT_Face p_face;
		QString glyphname ( int index );
		QByteArray GPOSTableRaw;
		QMap<quint16,QList<quint16> > coverages;
		QMap<quint16, QMap<quint16, double> > pairs;

		void makeCoverage();
		void makePairs ( quint16 subtableOffset );

		typedef QMap<quint16, QList<quint16> > ClassDefTable; // <Class , list<GLyph> >
		ClassDefTable getClass ( quint16 classDefOffset, quint16 coverageId );

		// return a uint16 from position index in GPOSTableRaw
		inline quint16 toUint16 ( quint16 index );
		// return a int16 from position index in GPOSTableRaw
		inline qint16 toInt16 ( quint16 index );
		/*
		0x0001 	XPlacement 	Includes horizontal adjustment for placement
		0x0002 	YPlacement 	Includes vertical adjustment for placement
		0x0004 	XAdvance 	Includes horizontal adjustment for advance
		0x0008 	YAdvance 	Includes vertical adjustment for advance
		0x0010 	XPlaDevice 	Includes horizontal Device table for placement
		0x0020 	YPlaDevice 	Includes vertical Device table for placement
		0x0040 	XAdvDevice 	Includes horizontal Device table for advance
		0x0080 	YAdvDevice 	Includes vertical Device table for advance
		0xF000 	Reserved 	For future use
		*/
		enum ValueFormat
		{
			XPlacement = 0x0001,
			YPlacement = 0x0002,
			XAdvance = 0x0004,
			YAdvance = 0x0008,
			XPlaDevice = 0x0010,
			YPlaDevice = 0x0020,
			XAdvDevice = 0x0040,
			YAdvDevice = 0x0080
		};
};


#endif // FMKERNFEAT_H


