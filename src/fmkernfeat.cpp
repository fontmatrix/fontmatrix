//
// C++ Implementation: fmkernfeat
//
// Description: try to make it the more compact & simple as possible
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmkernfeat.h"

#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include <QDebug>


FMKernFeature::FMKernFeature ( FT_Face face )
{
	FT_ULong length = 0;
	if ( !FT_Load_Sfnt_Table ( face, TTAG_GPOS , 0, NULL, &length ) )
	{
		if ( length > 32 )
		{
			GPOSTableRaw.resize ( length );
			FT_Load_Sfnt_Table ( face, TTAG_GPOS, 0, ( FT_Byte * ) GPOSTableRaw.data (), &length );
			
			makeCoverage();
			qDebug()<<"Coverage:"<<coverage.count();
		}
		else
			GPOSTableRaw.clear();
	}
}

FMKernFeature::~ FMKernFeature()
{
}


void FMKernFeature::makeCoverage()
{
	if ( GPOSTableRaw.isEmpty() )
		return;
	
	bool out(true);
	quint16 FeatureList_Offset= toUint16 ( 6 );
	quint16 LookupList_Offset = toUint16 ( 8 );

	// Find the offsets of the kern feature tables
	quint16 FeatureCount = toUint16 ( FeatureList_Offset );;
	QList<quint16> FeatureKern_Offset;
	for ( quint16 FeatureRecord ( 0 ); FeatureRecord < FeatureCount; ++ FeatureRecord )
	{
		int rawIdx ( FeatureList_Offset + 2 + ( 6 * FeatureRecord ) );
		quint32 tag ( FT_MAKE_TAG ( GPOSTableRaw.at ( rawIdx ),
		                            GPOSTableRaw.at ( rawIdx + 1 ),
		                            GPOSTableRaw.at ( rawIdx + 2 ),
		                            GPOSTableRaw.at ( rawIdx + 3 ) ) );
		if ( tag == TTAG_kern )
		{
			FeatureKern_Offset << ( toUint16 ( rawIdx + 4 ) + FeatureList_Offset );
			if(out)
			{
				qDebug()<<"KERN"<<FeatureKern_Offset;
			}
			
		}
	}
	
	// Extract indices of lookups for feture kern
	QList<quint16> LookupListIndex;
	foreach(quint16 kern, FeatureKern_Offset)
	{
		quint16 LookupCount( toUint16 ( kern + 2) );
		if(out)
		{
			qDebug()<<"\tParams"<<toUint16 ( kern );
			qDebug()<<"\tLookupCount"<<LookupCount;
		}
		for ( int llio ( 0 ) ; llio < LookupCount; ++llio )
		{
			quint16 Idx ( toUint16 (kern + 4 + ( llio * 2 ) ));
			if(!LookupListIndex.contains(Idx))
			{
				LookupListIndex <<Idx ;
			}
		}
		if(out)
			qDebug()<<"\tLookupIndex"<<LookupListIndex;
	}
	
	
	// Extract offsets of lookup tables for feature kern
	QList<quint16> LookupTables;
	QList<quint16> PairAdjustmentSubTables;
	for ( int i ( 0 ); i < LookupListIndex.count(); ++i )
	{
		int rawIdx( LookupList_Offset + 2 + ( LookupListIndex[i] * 2 ) );
		quint16 Lookup ( toUint16 ( rawIdx )  + LookupList_Offset );
		quint16 SubTableCount( toUint16(Lookup + 4) );
		for(int stIdx(0); stIdx < SubTableCount; ++ stIdx)
		{
			quint16 SubTable(toUint16(Lookup + 6 + (2 * stIdx)) + Lookup);

			quint16 PosFormat ( toUint16 ( SubTable ) );
			quint16 Coverage_Offset ( toUint16 ( SubTable + 2 ) + SubTable );
			quint16 CoverageFormat ( toUint16 ( Coverage_Offset ) );
		
			if(out)
			{
				qDebug()<<"\t\tPosFormat"<<PosFormat;
				qDebug()<<"\t\tCoverageFormat"<<CoverageFormat;
			}
			if ( 1 == CoverageFormat ) // glyph indices based
			{
				quint16 GlyphCount ( toUint16 ( Coverage_Offset + 2 ) );
				quint16 GlyphID ( Coverage_Offset + 4 );
				if(out)
					qDebug()<<"\t\t\tGlyphCount"<<GlyphCount;
				for ( unsigned int gl ( 0 ); gl < GlyphCount; ++gl )
				{
					coverage << toUint16 ( GlyphID + ( gl * 2 ) );
				}
			}
			else if( 2 == CoverageFormat) // Coverage Format2 => ranges based
			{
				quint16 RangeCount ( toUint16 ( Coverage_Offset + 2 ) );
				if(out)
					qDebug()<<"\t\t\tRangeCount" <<RangeCount;
				int gl_base ( 0 );
				for ( int r ( 0 ); r < RangeCount; ++r )
				{
					quint16 rBase ( Coverage_Offset + 2 + ( r * 6 ) );
					quint16 Start ( toUint16 ( rBase ) );
					quint16 End ( toUint16 ( rBase + 2 ) );
					quint16 StartCoverageIndex ( toUint16 ( rBase + 4 ) );
					for ( unsigned int gl ( Start ); gl <= End; ++gl )
						coverage << gl;
				}
			}
			else
				qDebug()<<"Unknow Coverage Format:"<<CoverageFormat;
		}
		
	}

}

quint16 FMKernFeature::toUint16(int index)
{
	if((index + 2) >= GPOSTableRaw.count() )
	{
		qDebug()<< "HORROR!" << index << GPOSTableRaw.count() ;
		return 0;
	}
	quint16 c1(GPOSTableRaw.at(index));
	quint16 c2(GPOSTableRaw.at(index + 1));
	c1 &= 0xFF;
	c2 &= 0xFF;
	quint16 ret((c1 << 8) | c2 );
// 	qDebug()<<"**"<<index<<"("<<c1 << c2 <<")"<<ret;
	return ret;
}
