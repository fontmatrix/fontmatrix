//
// C++ Implementation: fmshaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fmshaper.h"

#include <QList>
#include <QMap>
#include <QDebug>
#include <QVarLengthArray>

namespace Harfbuzz
{
HB_GraphemeClass HB_GetGraphemeClass(HB_UChar32 ch)
{
	return (HB_GraphemeClass) 0;
}
HB_WordClass HB_GetWordClass(HB_UChar32 ch)
{
	return (HB_WordClass) 0;
}
HB_SentenceClass HB_GetSentenceClass(HB_UChar32 ch)
{
	return (HB_SentenceClass) 0;
}

void HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *grapheme, HB_LineBreakClass *lineBreak)
{
	//###
}

static HB_Error hb_getSFntTable(void *font, HB_Tag tableTag, HB_Byte *buffer, HB_UInt *length)
{
	FT_Face face = (FT_Face)font;
	FT_ULong ftlen = *length;
	FT_Error error = 0;

	if (!FT_IS_SFNT(face))
		return HB_Err_Invalid_Argument;

	error = FT_Load_Sfnt_Table(face, tableTag, 0, buffer, &ftlen);
	*length = ftlen;
	return (HB_Error)error;
}
}

/***************** UTILS ************/
namespace 
{	
	Harfbuzz::HB_Script script2script ( QString script )
	{
		QMap<QString, Harfbuzz::HB_Script> hbscmap;
		hbscmap["arab"] = Harfbuzz::HB_Script_Arabic ;
		hbscmap["armn"] = Harfbuzz::HB_Script_Armenian ;
		hbscmap["beng"] = Harfbuzz::HB_Script_Bengali ;
		hbscmap["cyrl"] = Harfbuzz::HB_Script_Cyrillic ;
		hbscmap["deva"] = Harfbuzz::HB_Script_Devanagari ;
		hbscmap["geor"] = Harfbuzz::HB_Script_Georgian ;
		hbscmap["grek"] = Harfbuzz::HB_Script_Greek ;
		hbscmap["gujr"] = Harfbuzz::HB_Script_Gujarati ;
		hbscmap["guru"] = Harfbuzz::HB_Script_Gurmukhi ;
		hbscmap["hang"] = Harfbuzz::HB_Script_Hangul ;
		hbscmap["hebr"] = Harfbuzz::HB_Script_Hebrew ;
		hbscmap["knda"] = Harfbuzz::HB_Script_Kannada ;
		hbscmap["khmr"] = Harfbuzz::HB_Script_Khmer ;
		hbscmap["lao "] = Harfbuzz::HB_Script_Lao ;
		hbscmap["mlym"] = Harfbuzz::HB_Script_Malayalam ;
		hbscmap["mymr"] = Harfbuzz::HB_Script_Myanmar ;
		hbscmap["ogam"] = Harfbuzz::HB_Script_Ogham ;
		hbscmap["orya"] = Harfbuzz::HB_Script_Oriya ;
		hbscmap["runr"] = Harfbuzz::HB_Script_Runic ;
		hbscmap["sinh"] = Harfbuzz::HB_Script_Sinhala ;
		hbscmap["syrc"] = Harfbuzz::HB_Script_Syriac ;
		hbscmap["taml"] = Harfbuzz::HB_Script_Tamil ;
		hbscmap["telu"] = Harfbuzz::HB_Script_Telugu ;
		hbscmap["thaa"] = Harfbuzz::HB_Script_Thaana ;
		hbscmap["thai"] = Harfbuzz::HB_Script_Thai ;
		hbscmap["tibt"] = Harfbuzz::HB_Script_Tibetan ;
	
		Harfbuzz::HB_Script ret = hbscmap.contains ( script ) ? hbscmap[script] : Harfbuzz::HB_Script_Common;
		return   ret;
	}

}

FMShaper::FMShaper(FMOtf *anchor)
	:anchorOTF(anchor)
{
	faceisset = langisset = allocated = false;
	setFont();
	qDebug() << "FMShaper "<< this <<" created";
}

FMShaper::~ FMShaper()
{
	qDebug() << "FMShaper "<< this <<" destructor";
	if (faceisset)
	{
		if(m.font)
		{
			qDebug() << "Freeing m.font at "<< m.font;
			delete m.font;
		}
		if(m.face)
		{
			qDebug() << "Freeing m.face at "<< m.face;
			HB_FreeFace(m.face);
		}
	}
	qDebug() << "FMShaper "<< this <<" destroyed";
}

bool FMShaper::setFont (/*FT_Face face, Harfbuzz::HB_Font font  */)
{
	Harfbuzz::HB_Face hbFace = Harfbuzz::HB_NewFace(anchorOTF->_face,Harfbuzz::hb_getSFntTable);
	Harfbuzz::HB_Font hbFont = new Harfbuzz::HB_FontRec;
	
	
	hbFont->klass = anchorOTF->hbFont.klass ;
	hbFont->userData = anchorOTF->hbFont.userData;
	hbFont->x_ppem  = anchorOTF->hbFont.x_ppem;
	hbFont->y_ppem  = anchorOTF->hbFont.y_ppem;
	hbFont->x_scale = anchorOTF->hbFont.x_scale;
	hbFont->y_scale = anchorOTF->hbFont.y_scale;
	m.font = hbFont;
	m.face = hbFace;
	
	anchorFace = anchorOTF->_face;
	faceisset = true;
	return faceisset;
}


bool FMShaper::setScript ( QString script )
{
	Harfbuzz::HB_Script ret = script2script ( script );
	if ( ret != Harfbuzz::HB_Script_Common )
	{
		m.item.script = ret;
		return true;
	}
	return false;
}

QList< RenderedGlyph > FMShaper::doShape(QString string, bool ltr)
{
	qDebug() << "FMShaper::doShape("<<string<<","<<ltr<<")";
	
	if(!faceisset)
		setFont();
	
	m.kerning_applied = false;
	m.string = reinterpret_cast<const Harfbuzz::HB_UChar16 *> ( string.constData() );
	m.stringLength = string.length();
	m.item.pos = 0;
	m.item.bidiLevel =  0;
	m.shaperFlags = Harfbuzz::HB_ShaperFlag_UseDesignMetrics;
	
	m.initialGlyphCount = m.num_glyphs = m.item.length = m.stringLength;
	m.glyphIndicesPresent = false;

	int neededspace = m.num_glyphs  ;

	QVarLengthArray<Harfbuzz::HB_Glyph> hb_glyphs(neededspace);
	QVarLengthArray<Harfbuzz::HB_GlyphAttributes> hb_attributes(neededspace);
	QVarLengthArray<Harfbuzz::HB_Fixed> hb_advances(neededspace);
	QVarLengthArray<Harfbuzz::HB_FixedPoint> hb_offsets(neededspace);
	QVarLengthArray<unsigned short> hb_logClusters(neededspace);

	Harfbuzz::HB_Bool result = false;
	int iter = 0;
	while ( !result )
	{
		neededspace = m.num_glyphs  ;

		hb_glyphs.resize(neededspace);
		hb_attributes.resize(neededspace);
		hb_advances.resize(neededspace);
		hb_offsets.resize(neededspace);
		hb_logClusters.resize(neededspace);

		memset(hb_glyphs.data(), 0, hb_glyphs.size() * sizeof(Harfbuzz::HB_Glyph));
		memset(hb_attributes.data(), 0, hb_attributes.size() * sizeof(Harfbuzz::HB_GlyphAttributes));
		memset(hb_advances.data(), 0, hb_advances.size() * sizeof(Harfbuzz::HB_Fixed));
		memset(hb_offsets.data(), 0, hb_offsets.size() * sizeof(Harfbuzz::HB_FixedPoint));
		memset(hb_logClusters.data(), 0, hb_logClusters.size() * sizeof(unsigned short));

		m.glyphs = hb_glyphs.data();
		m.attributes = hb_attributes.data();
		m.advances = hb_advances.data();
		m.offsets = hb_offsets.data();
		m.log_clusters = hb_logClusters.data();
		
		qDebug() << "----------------------------------------------item allocated------------";
		result = Harfbuzz::HB_ShapeItem ( &m );
		qDebug() << "----------------------------------------------ShapeItem run"<<++iter<<" - "<< (result ? "has " : "wants ") << m.num_glyphs <<" glyphs-";
	}
	
	
	QList<RenderedGlyph> renderedString;
	int base = 0;
	int baseCorrection = 0;
	for(int gIndex = 0; gIndex < m.num_glyphs; ++gIndex)
	{
		Harfbuzz::HB_GlyphAttributes attr = m.attributes[gIndex];
		qDebug()<< "ATTR("<< m.glyphs[gIndex] 
				<< ") combiningClass = " << attr.combiningClass
				<< "; clusterStart =" << attr.clusterStart
				<< "; mark = "<< attr.mark;
		if(m.attributes[gIndex].clusterStart )
		{
			base = gIndex;
			baseCorrection = 0;
		}
// 		if(m.attributes[gIndex].mark )
		else
		{
			qDebug() << "catch a mark";
// 			for(int b=base; b < gIndex; ++b)
			{
// 				baseCorrection = renderedString[base].xadvance;
			}
		}
			RenderedGlyph gl;
			gl.glyph = m.glyphs[gIndex];
			gl.xadvance = ltr ? ( double ) ( m.advances[gIndex]) :( double ) ( -m.advances[gIndex]) ;
			gl.yadvance = 0.0;
			gl.xoffset = ltr ? ( m.offsets[gIndex].x  - baseCorrection ) : ( baseCorrection - m.offsets[gIndex].x );
			gl.yoffset = m.offsets[gIndex].y ;
				
			renderedString << gl;
	}
	qDebug() << "EndOf FMShaper::doShape("<<string<<","<<ltr<<")";
	return renderedString;
}



Harfbuzz::HB_Buffer  FMShaper::out_buffer()
{
	return m.face->buffer;
}




