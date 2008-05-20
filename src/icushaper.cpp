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

#include "icushaper.h"

#include <iostream>
using std::cerr; 


IcuShaper::IcuShaper ( FMOtf * o, QString s )
		:FMBaseShaper ( o,s )
{
	LEErrorCode err;
	icuFont = new IcuFontImpl ( otf );
	icuLE = LayoutEngine::layoutEngineFactory ( icuFont, tagToCode.value ( script ), 0 , err );
}

IcuShaper::~ IcuShaper()
{
	icuLE->reset();
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
		// We won’t use language at first
// 		tagToCode["ARA"] = araLanguageTag  ;
// 		tagToCode["ASM"] = asmLanguageTag  ;
// 		tagToCode["BEN"] = benLanguageTag  ;
// 		tagToCode["FAR"] = farLanguageTag  ;
// 		tagToCode["GUJ"] = gujLanguageTag  ;
// 		tagToCode["HIN"] = hinLanguageTag  ;
// 		tagToCode["IWR"] = iwrLanguageTag  ;
// 		tagToCode["JII"] = jiiLanguageTag  ;
// 		tagToCode["JAN"] = janLanguageTag  ;
// 		tagToCode["KAN"] = kanLanguageTag  ;
// 		tagToCode["KOK"] = kokLanguageTag  ;
// 		tagToCode["KOR"] = korLanguageTag  ;
// 		tagToCode["KSH"] = kshLanguageTag  ;
// 		tagToCode["MAL"] = malLanguageTag  ;
// 		tagToCode["MAR"] = marLanguageTag  ;
// 		tagToCode["MLR"] = mlrLanguageTag  ;
// 		tagToCode["MNI"] = mniLanguageTag  ;
// 		tagToCode["ORI"] = oriLanguageTag  ;
// 		tagToCode["SAN"] = sanLanguageTag  ;
// 		tagToCode["SND"] = sndLanguageTag  ;
// 		tagToCode["SNH"] = snhLanguageTag  ;
// 		tagToCode["SYR"] = syrLanguageTag  ;
// 		tagToCode["TAM"] = tamLanguageTag  ;
// 		tagToCode["TEL"] = telLanguageTag  ;
// 		tagToCode["THA"] = thaLanguageTag  ;
// 		tagToCode["URD"] = urdLanguageTag  ;
// 		tagToCode["ZHP"] = zhpLanguageTag  ;
// 		tagToCode["ZHS"] = zhsLanguageTag  ;
// 		tagToCode["ZHT"] = zhtLanguageTag  ;

	}
}


GlyphList IcuShaper::doShape ( const QString & s )
{
	GlyphList ret;
	int glAllocated ( 0 );

	LEErrorCode err;

	LEUnicode16 *ts = new LEUnicode[s.count() ];
	for ( int i ( 0 ); i< s.count();++i )
	{
		ts[i] =  s[i].unicode();
		cerr << "["<< s[i].unicode() << "]";
	}
	cerr<< std::endl;

	icuLE->reset();
	glAllocated = icuLE->layoutChars ( ts, 0, s.count(), s.count(), false, 0, 0, err ) ;
	
	if(err)
		cerr<<"ICU ERROR ("<< err <<")"<<std::endl;

	LEGlyphID *glyphs    = new LEGlyphID[glAllocated];
	le_int32 *indices   = new le_int32[glAllocated];
	float     *positions = new float[ ( glAllocated * 2 ) + 2];
	icuLE->getGlyphs(glyphs, err);
	icuLE->getCharIndices(indices, err);
	icuLE->getGlyphPositions(positions, err);
	float stackX(0.0);

	for ( int gIdx ( 0 ); gIdx < glAllocated ; ++gIdx )
	{
		RenderedGlyph rg;
		rg.glyph = glyphs[gIdx];
		rg.log = indices[gIdx];
		rg.xadvance = positions[(gIdx + 1) * 2] - stackX;
		rg.yadvance = positions[((gIdx + 1) * 2) + 1];
		rg.xoffset = rg.yoffset = 0;
		stackX +=  rg.xadvance;
		
		ret << rg;
		
		cerr<< "["<< rg.log <<"]";
		cerr<< "["<< rg.glyph <<"]";
		cerr<< "["<< rg.xadvance <<"]";
		cerr<< "["<< positions[gIdx * 2] <<"]";
		cerr<< std::endl;
	}
	icuLE->reset();
	
	delete  ts;
	delete  glyphs;
	delete  indices;
	delete  positions;
	
	return ret;
}


/// Je suis furieux d’avoir à écrire ça - pm

IcuFontImpl::IcuFontImpl ( FMOtf * o )
		:otf ( o )
{
}

IcuFontImpl::~ IcuFontImpl()
{

}

const void * IcuFontImpl::getFontTable ( LETag tableTag ) const
{
	cerr<< "IcuFontImpl::getFontTable" <<std::endl;
	FT_Face face ( otf->face() );
	FT_ULong length(0);
	if ( !FT_Load_Sfnt_Table ( face, tableTag, 0, NULL, &length ) )
	{
		if ( length > 0 )
		{
			FT_Byte * bA = new FT_Byte[length];
			
			FT_Load_Sfnt_Table ( face, tableTag, 0, bA, &length );
			
			return ( const void* ) bA;
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
	cerr << "IcuFontImpl::mapCharToGlyph("<< ch <<") = "<<gi<<std::endl;
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
	cerr<< "IcuFontImpl::getGlyphPoint" <<std::endl;
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


