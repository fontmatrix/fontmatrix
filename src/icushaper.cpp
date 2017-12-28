//
// C++ Implementation: icushaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QDebug>
#include "icushaper.h"

#include <iostream>
using std::cerr; 

QString OTF_tag_name ( unsigned int tag );
unsigned int OTF_name_tag ( QString s );

IcuShaper::IcuShaper ( FMOtf * o, QString s )
		:FMBaseShaper ( o,s )
{
	fillTagToCode();
	qDebug()<<"IcuShaper::IcuShaper("<< tagToCode.value ( script ) <<")";
	LEErrorCode err(LE_NO_ERROR);
	icuFont = new IcuFontImpl ( otf );
	icuLE = LayoutEngine::layoutEngineFactory ( icuFont,  tagToCode.value ( script ), -1 , err );
	IcuError(err);
}

IcuShaper::~ IcuShaper()
{
	icuLE->reset();
	delete icuLE;
	delete icuFont;
}

void IcuShaper::fillTagToCode()
{
	if ( tagToCode.isEmpty() )
	{
		tagToCode["zyyy"] = zyyyScriptCode  ;
		tagToCode["qaai"] = qaaiScriptCode  ;
		tagToCode["arab"] = arabScriptCode  ;
		tagToCode["armn"] = armnScriptCode  ;
		tagToCode["beng"] = bengScriptCode  ;
		tagToCode["bopo"] = bopoScriptCode  ;
		tagToCode["cher"] = cherScriptCode  ;
		tagToCode["copt"] = coptScriptCode  ;
		tagToCode["cyrl"] = cyrlScriptCode  ;
		tagToCode["dsrt"] = dsrtScriptCode  ;
		tagToCode["deva"] = devaScriptCode  ;
		tagToCode["ethi"] = ethiScriptCode  ;
		tagToCode["geor"] = georScriptCode  ;
		tagToCode["goth"] = gothScriptCode  ;
		tagToCode["grek"] = grekScriptCode  ;
		tagToCode["gujr"] = gujrScriptCode  ;
		tagToCode["guru"] = guruScriptCode  ;
		tagToCode["hani"] = haniScriptCode  ;
		tagToCode["hang"] = hangScriptCode  ;
		tagToCode["hebr"] = hebrScriptCode  ;
		tagToCode["hira"] = hiraScriptCode  ;
		tagToCode["knda"] = kndaScriptCode  ;
		tagToCode["kana"] = kanaScriptCode  ;
		tagToCode["khmr"] = khmrScriptCode  ;
		tagToCode["lao "] = laooScriptCode  ;
		tagToCode["latn"] = latnScriptCode  ;
		tagToCode["mlym"] = mlymScriptCode  ;
		tagToCode["mong"] = mongScriptCode  ;
		tagToCode["mymr"] = mymrScriptCode  ;
		tagToCode["ogam"] = ogamScriptCode  ;
		tagToCode["ital"] = italScriptCode  ;
		tagToCode["orya"] = oryaScriptCode  ;
		tagToCode["runr"] = runrScriptCode  ;
		tagToCode["sinh"] = sinhScriptCode  ;
		tagToCode["syrc"] = syrcScriptCode  ;
		tagToCode["taml"] = tamlScriptCode  ;
		tagToCode["telu"] = teluScriptCode  ;
		tagToCode["thaa"] = thaaScriptCode  ;
		tagToCode["thai"] = thaiScriptCode  ;
		tagToCode["tibt"] = tibtScriptCode  ;
		tagToCode["cans"] = cansScriptCode  ;
		tagToCode["yi  "] = yiiiScriptCode  ;
		tagToCode["tglg"] = tglgScriptCode  ;
		tagToCode["hano"] = hanoScriptCode  ;
		tagToCode["buhd"] = buhdScriptCode  ;
		tagToCode["tagb"] = tagbScriptCode  ;
		tagToCode["brai"] = braiScriptCode  ;
		tagToCode["cprt"] = cprtScriptCode  ;
		tagToCode["limb"] = limbScriptCode  ;
		tagToCode["linb"] = linbScriptCode  ;
		tagToCode["osma"] = osmaScriptCode  ;
		tagToCode["shaw"] = shawScriptCode  ;
		tagToCode["tale"] = taleScriptCode  ;
		tagToCode["ugar"] = ugarScriptCode  ;
		tagToCode["hrkt"] = hrktScriptCode  ;
		tagToCode["bugi"] = bugiScriptCode  ;
		tagToCode["glag"] = glagScriptCode  ;
		tagToCode["khar"] = kharScriptCode  ;
		tagToCode["sylo"] = syloScriptCode  ;
		tagToCode["talu"] = taluScriptCode  ;
		tagToCode["tfng"] = tfngScriptCode  ;
		tagToCode["xpeo"] = xpeoScriptCode  ;
		tagToCode["bali"] = baliScriptCode  ;
		tagToCode["batk"] = batkScriptCode  ;
		tagToCode["blis"] = blisScriptCode  ;
		tagToCode["brah"] = brahScriptCode  ;
		tagToCode["cham"] = chamScriptCode  ;
		tagToCode["cirt"] = cirtScriptCode  ;
		tagToCode["cyrs"] = cyrsScriptCode  ;
		tagToCode["egyd"] = egydScriptCode  ;
		tagToCode["egyh"] = egyhScriptCode  ;
		tagToCode["egyp"] = egypScriptCode  ;
		tagToCode["geok"] = geokScriptCode  ;
		tagToCode["hans"] = hansScriptCode  ;
		tagToCode["hant"] = hantScriptCode  ;
		tagToCode["hmng"] = hmngScriptCode  ;
		tagToCode["hung"] = hungScriptCode  ;
		tagToCode["inds"] = indsScriptCode  ;
		tagToCode["java"] = javaScriptCode  ;
		tagToCode["kali"] = kaliScriptCode  ;
		tagToCode["latf"] = latfScriptCode  ;
		tagToCode["latg"] = latgScriptCode  ;
		tagToCode["lepc"] = lepcScriptCode  ;
		tagToCode["lina"] = linaScriptCode  ;
		tagToCode["mand"] = mandScriptCode  ;
		tagToCode["maya"] = mayaScriptCode  ;
		tagToCode["mero"] = meroScriptCode  ;
		tagToCode["nko "] = nkooScriptCode  ;
		tagToCode["orkh"] = orkhScriptCode  ;
		tagToCode["perm"] = permScriptCode  ;
		tagToCode["phag"] = phagScriptCode  ;
		tagToCode["phnx"] = phnxScriptCode  ;
		tagToCode["plrd"] = plrdScriptCode  ;
		tagToCode["roro"] = roroScriptCode  ;
		tagToCode["sara"] = saraScriptCode  ;
		tagToCode["syre"] = syreScriptCode  ;
		tagToCode["syrj"] = syrjScriptCode  ;
		tagToCode["syrn"] = syrnScriptCode  ;
		tagToCode["teng"] = tengScriptCode  ;
		tagToCode["vai "] = vaiiScriptCode  ;
		tagToCode["visp"] = vispScriptCode  ;
		tagToCode["xsux"] = xsuxScriptCode  ;
		tagToCode["zxxx"] = zxxxScriptCode  ;
		tagToCode["zzzz"] = zzzzScriptCode  ;
	}
}


GlyphList IcuShaper::doShape ( const QString & s )
{
	GlyphList ret;
	int glAllocated ( 0 );

	LEErrorCode err(LE_NO_ERROR);

	LEUnicode16 *ts = new LEUnicode[s.count() ];
	for ( int i ( 0 ); i< s.count();++i )
	{
		ts[i] =  s[i].unicode();
// 		cerr << "["<< s[i].unicode() << "]";
	}
// 	cerr<< std::endl;

	icuLE->reset();
	int sCount (s.count());
	glAllocated = icuLE->layoutChars ( ts, 0,sCount , sCount, false, 0, 0, err ) ;
	IcuError(err);
	
// 	cerr<<"provided "<<s.count()<< " got "<< glAllocated<< std::endl;

	LEGlyphID *glyphs    = new LEGlyphID[glAllocated];
	le_int32 *indices   = new le_int32[glAllocated];
	float     *positions = new float[ ( glAllocated * 2 ) + 2];
	icuLE->getGlyphs ( glyphs, err );
	icuLE->getCharIndices ( indices, err );
	icuLE->getGlyphPositions ( positions, err );
	float stackX ( 0.0 );

	for ( int gIdx ( 0 ); gIdx < glAllocated ; ++gIdx )
	{
		if(glyphs[gIdx] != 0xFFFF)
		{
			RenderedGlyph rg;
			rg.glyph = glyphs[gIdx];
			rg.log = indices[gIdx];
			rg.lChar = s[rg.log].unicode();
			rg.xadvance = positions[ ( gIdx + 1 ) * 2] - stackX;
			rg.yadvance = positions[ ( ( gIdx + 1 ) * 2 ) + 1];
			rg.xoffset = rg.yoffset = 0;
			stackX +=  rg.xadvance;
	
			ret << rg;
		}

// 		cerr<< "["<< indices[gIdx] <<"]";
// 		cerr<< "["<< glyphs[gIdx] <<"]";
// 		cerr<< std::endl;
	}
	icuLE->reset();

	delete  ts;
	delete  glyphs;
	delete  indices;
	delete  positions;

	return ret;
}


void IcuShaper::IcuError(int err)
{
	switch ( err )
	{
		case LE_NO_SUBFONT_WARNING  : qDebug()<<"The font does not contain subfonts.";break;
// 		case LE_NO_ERROR  : qDebug()<<"No error, no warning.";break;
		case LE_ILLEGAL_ARGUMENT_ERROR  : qDebug()<<"An illegal argument was detected.";break;
		case LE_MEMORY_ALLOCATION_ERROR  : qDebug()<<"Memory allocation error.";break;
		case LE_INDEX_OUT_OF_BOUNDS_ERROR  : qDebug()<<"Trying to access an index that is out of bounds.";break;
		case LE_NO_LAYOUT_ERROR  : qDebug()<<"You must call layoutChars() first.";break;
		case LE_INTERNAL_ERROR  : qDebug()<<"An internal error was encountered.";break;
		case LE_FONT_FILE_NOT_FOUND_ERROR  : qDebug()<<"The requested font file cannot be opened.";break;
		case LE_MISSING_FONT_TABLE_ERROR  : qDebug()<<"The requested font table does not exist.";break;
		default:/*qDebug()<<"NoCode"*/;
	}
}



/// Je suis furieux d’avoir à écrire ça - pm

IcuFontImpl *IcuFontImpl::instance = 0;

IcuFontImpl::IcuFontImpl ( FMOtf * o )
		:otf ( o )
{
	instance = this;
}

IcuFontImpl::~ IcuFontImpl()
{
	foreach(unsigned char* p, tables)
	{
		delete p;
	}

}

const void * IcuFontImpl::getFontTable ( LETag tableTag, size_t &length_sz ) const
{
// 	qDebug()<< "IcuFontImpl::getFontTable" << OTF_tag_name( tableTag );
	FT_Face face ( otf->face() );
	FT_ULong length(0);
	if ( !FT_Load_Sfnt_Table ( face, tableTag, 0, NULL, &length ) )
	{
		if ( length > 0 )
		{
// 			qDebug()<<"Table len"<< length;
			FT_Byte * bA = new FT_Byte[length];
			
			FT_Load_Sfnt_Table ( face, tableTag, 0, bA, &length );
			
			regTables( tableTag,  bA );
			return  (const void*) tables.value(tableTag);
		}

	}
	return 0;
}

le_int32 IcuFontImpl::getUnitsPerEM() const
{
	return otf->face()->units_per_EM;
}

LEGlyphID IcuFontImpl::mapCharToGlyph ( LEUnicode32 ch ) const
{
	int gi(FT_Get_Char_Index ( otf->face(), ch ));
// 	cerr << "IcuFontImpl::mapCharToGlyph("<< ch <<") = "<<gi<<std::endl;
	return gi;
}

void IcuFontImpl::getGlyphAdvance ( LEGlyphID glyph, LEPoint & advance ) const
{
	FT_Face face ( otf->face() );
	if ( !FT_Load_Glyph ( face, glyph , FT_LOAD_NO_SCALE ) )
	{
		advance.fX = face->glyph->metrics.horiAdvance;
		advance.fY = face->glyph->metrics.vertAdvance;
	}
	else
	{
		advance.fX = 0;
		advance.fY = 0;
	}
}

le_bool IcuFontImpl::getGlyphPoint ( LEGlyphID glyph, le_int32 pointNumber, LEPoint & point ) const
{
// 	cerr<< "IcuFontImpl::getGlyphPoint" <<std::endl;
	return false;
}

float IcuFontImpl::getXPixelsPerEm() const
{
	return getUnitsPerEM();
}

float IcuFontImpl::getYPixelsPerEm() const
{
	return getUnitsPerEM();
}

float IcuFontImpl::getScaleFactorX() const
{
	return 1.0;
}

float IcuFontImpl::getScaleFactorY() const
{
	return 1.0;
}

le_int32 IcuFontImpl::getAscent() const
{
	return otf->face()->ascender;
}

le_int32 IcuFontImpl::getDescent() const
{
	return otf->face()->descender;
}

le_int32 IcuFontImpl::getLeading() const
{
	return 0;
}

