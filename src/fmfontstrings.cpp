//
// C++ Implementation: fmfontstrings
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmfontstrings.h"
#include "fmpaths.h"

#include <QFile>

FontStrings * FontStrings::instance = 0;
FontStrings::FontStrings()
{
	fillNamesMeaning();
	fillPanoseMap();
	fillCharsetMap();
	fillTTTableList();
	fillFSftypeMap();
}

FontStrings * FontStrings::getInstance()
{
	if(!instance)
		instance = new FontStrings;
	return instance;
}

void FontStrings::fillNamesMeaning()
{
	m_name[FMFontDb::Copyright]= tr ( "Copyright" );
	m_name[FMFontDb::FontFamily]= tr ( "Font Family" );
	m_name[FMFontDb::FontSubfamily]= tr ( "Font Subfamily" );
	m_name[FMFontDb::UniqueFontIdentifier]= tr ( "Unique font identifier" );
	m_name[FMFontDb::FullFontName]= tr ( "Full font name" );
	m_name[FMFontDb::VersionString]= tr ( "Version string" );
	m_name[FMFontDb::PostscriptName]= tr ( "Postscript name" );
	m_name[FMFontDb::Trademark]= tr ( "Trademark" );
	m_name[FMFontDb::ManufacturerName]= tr ( "Manufacturer" );
	m_name[FMFontDb::Designer]= tr ( "Designer" );
	m_name[FMFontDb::Description]= tr ( "Description" );
	m_name[FMFontDb::URLVendor]= tr ( "URL Vendor" );
	m_name[FMFontDb::URLDesigner]= tr ( "URL Designer" );
	m_name[FMFontDb::LicenseDescription]= tr ( "License Description" );
	m_name[FMFontDb::LicenseInfoURL]= tr ( "License Info URL" );
	m_name[FMFontDb::PreferredFamily]= tr ( "Preferred Family" );
	m_name[FMFontDb::PreferredSubfamily]= tr ( "Preferred Subfamily" );
	m_name[FMFontDb::CompatibleMacintosh]= tr ( "Compatible Full (Macintosh only)" );
	m_name[FMFontDb::SampleText]= tr ( "Sample text" );
	m_name[FMFontDb::PostScriptCIDName]= tr ( "PostScript CID findfont name" );
// 	m_name[FMFontDb::Panose]= tr("Panose");
	m_name[FMFontDb::AllInfo]= tr("All fields");
	
}

void FontStrings::fillPanoseMap()
{
// 	QString panofilepath( FMPaths::LocalizedFilePath(FMPaths::ResourcesDir() + "Panose", ".xml" ) );
// 	if(!panofilepath.isEmpty())
// 	{
// 		panoseFromFile(panofilepath);
// 		return;
// 	}
		
	// http://www.microsoft.com/OpenType/OTSpec/os2ver0.htm#pan
	// http://www.monotypeimaging.com/ProductsServices/pan2.aspx

		QMap<int, QString> mapModel;
		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Text and Display","Family Type" ) ;
		mapModel[ 3 ] = tr ( "Script" ,"Family Type" ) ;
		mapModel[ 4 ] = tr ( "Decorative","Family Type"  ) ;
		mapModel[ 5 ] = tr ( "Pictorial" ,"Family Type" ) ;

		m_panoseMap[FamilyType] = mapModel;
		m_panoseKeyName[FamilyType] = tr ( "Family Type" );
		m_panoseKeyInfo[FamilyType] = "<h1>Family Kind</h1> Defines what type of font is being classified." ;
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Cove" ,"Serif style" ) ;
		mapModel[ 3 ] = tr ( "Obtuse Cove" ,"Serif style"  ) ;
		mapModel[ 4 ] = tr ( "Square Cove" ,"Serif style"  ) ;
		mapModel[ 5 ] = tr ( "Obtuse Square Cove"  ,"Serif style" ) ;
		mapModel[ 6 ] = tr ( "Square" ,"Serif style"  ) ;
		mapModel[ 7 ] = tr ( "Thin"  ,"Serif style" ) ;
		mapModel[ 8 ] = tr ( "Bone"  ,"Serif style" ) ;
		mapModel[ 9 ] = tr ( "Exaggerated"  ,"Serif style" ) ;
		mapModel[ 10 ] = tr ( "Triangle"  ,"Serif style" ) ;
		mapModel[ 11 ] = tr ( "Normal Sans"  ,"Serif style" ) ;
		mapModel[ 12 ] = tr ( "Obtuse Sans"  ,"Serif style" ) ;
		mapModel[ 13 ] = tr ( "Perp Sans" ,"Serif style"  ) ;
		mapModel[ 14 ] = tr ( "Flared"  ,"Serif style" ) ;
		mapModel[ 15 ] = tr ( "Rounded"  ,"Serif style" ) ;

		m_panoseMap[SerifStyle ] = mapModel;
		m_panoseKeyName[SerifStyle] = tr ( "Serif style" ) ;
		m_panoseKeyInfo[SerifStyle] ="<h1>Serif style</h1>This digit describes the appearance of the serifs used in a font design";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Very Light" ,"Weight" ) ;
		mapModel[ 3 ] = tr ( "Light" ,"Weight") ;
		mapModel[ 4 ] = tr ( "Thin" ,"Weight") ;
		mapModel[ 5 ] = tr ( "Book" ,"Weight") ;
		mapModel[ 6 ] = tr ( "Medium" ,"Weight") ;
		mapModel[ 7 ] = tr ( "Demi" ,"Weight") ;
		mapModel[ 8 ] = tr ( "Bold" ,"Weight") ;
		mapModel[ 9 ] = tr ( "Heavy" ,"Weight") ;
		mapModel[ 10 ] = tr ( "Black" ,"Weight") ;
		mapModel[ 11 ] = tr ( "Nord" ,"Weight") ;

		m_panoseMap[Weight ] = mapModel;
		m_panoseKeyName[Weight] = tr ( "Weight" );
		m_panoseKeyInfo[Weight] ="<h1>Weight</h1>The Weight digit classifies the appearance of a fonts’ stroke thickness in relation to its height.";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Old Style" ,"Proportion") ;
		mapModel[ 3 ] = tr ( "Modern" ,"Proportion" ) ;
		mapModel[ 4 ] = tr ( "Even Width" ,"Proportion" ) ;
		mapModel[ 5 ] = tr ( "Expanded" ,"Proportion" ) ;
		mapModel[ 6 ] = tr ( "Condensed"  ,"Proportion") ;
		mapModel[ 7 ] = tr ( "Very Expanded"  ,"Proportion") ;
		mapModel[ 8 ] = tr ( "Very Condensed"  ,"Proportion") ;
		mapModel[ 9 ] = tr ( "Monospaced"  ,"Proportion") ;

		m_panoseMap[Proportion ] = mapModel;
		m_panoseKeyName[Proportion] = tr ( "Proportion" );
		m_panoseKeyInfo[Proportion] ="<h1>Proportion</h1>The proportion of a font in the PANOSE Typeface Matching System is defined in greater detail than simply an indication of general glyph shape aspect ratio such as extended and condensed.";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "None" ,"Contrast") ;
		mapModel[ 3 ] = tr ( "Very Low" ,"Contrast") ;
		mapModel[ 4 ] = tr ( "Low" ,"Contrast") ;
		mapModel[ 5 ] = tr ( "Medium Low" ,"Contrast") ;
		mapModel[ 6 ] = tr ( "Medium" ,"Contrast") ;
		mapModel[ 7 ] = tr ( "Medium High" ,"Contrast") ;
		mapModel[ 8 ] = tr ( "High" ,"Contrast") ;
		mapModel[ 9 ] = tr ( "Very High" ,"Contrast") ;

		m_panoseMap[Contrast ] = mapModel;
		m_panoseKeyName[Contrast] = tr ( "Contrast" );
		m_panoseKeyInfo[Contrast] ="<h1>Contrast</h1>The Contrast digit describes the ratio between the thickest point on the stroke of the letter O and the narrowest point on the letter O.";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Gradual/Diagonal" , "Stroke Variation") ;
		mapModel[ 3 ] = tr ( "Gradual/Transitional"  , "Stroke Variation") ;
		mapModel[ 4 ] = tr ( "Gradual/Vertical" , "Stroke Variation" ) ;
		mapModel[ 5 ] = tr ( "Gradual/Horizontal" , "Stroke Variation" ) ;
		mapModel[ 6 ] = tr ( "Rapid/Vertical"  , "Stroke Variation") ;
		mapModel[ 7 ] = tr ( "Rapid/Horizontal"  , "Stroke Variation") ;
		mapModel[ 8 ] = tr ( "Instant/Vertical"  , "Stroke Variation") ;

		m_panoseMap[StrokeVariation] = mapModel;
		m_panoseKeyName[StrokeVariation] = tr ( "Stroke Variation" );
		m_panoseKeyInfo[StrokeVariation] ="<h1>Stroke Variation</h1>The Stroke Variation category further details the contrast trait by describing the kind of transition that occurs as the stem thickness changes on rounded glyph shapes.";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Straight Arms/Horizontal" ,"Arm Style") ;
		mapModel[ 3 ] = tr ( "Straight Arms/Wedge" ,"Arm Style") ;
		mapModel[ 4 ] = tr ( "Straight Arms/Vertical" ,"Arm Style") ;
		mapModel[ 5 ] = tr ( "Straight Arms/Single Serif","Arm Style" ) ;
		mapModel[ 6 ] = tr ( "Straight Arms/Double Serif","Arm Style" ) ;
		mapModel[ 7 ] = tr ( "Non-Straight Arms/Horizontal" ,"Arm Style") ;
		mapModel[ 8 ] = tr ( "Non-Straight Arms/Wedge" ,"Arm Style") ;
		mapModel[ 9 ] = tr ( "Non-Straight Arms/Vertical" ,"Arm Style") ;
		mapModel[ 10 ] = tr ( "Non-Straight Arms/Single Serif","Arm Style" ) ;
		mapModel[ 11 ] = tr ( "Non-Straight Arms/Double Serif" ,"Arm Style") ;

		m_panoseMap[ArmStyle ] = mapModel;
		m_panoseKeyName[ArmStyle] = tr ( "Arm Style" );
		m_panoseKeyInfo[ArmStyle] ="<h1>Arm Style</h1>The Arm Style category classifies two attributes of a glyph design: special treatment of diagonal stems and termination of open rounded letterforms.";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Normal/Contact" , "Letterform" ) ;
		mapModel[ 3 ] = tr ( "Normal/Weighted" , "Letterform" ) ;
		mapModel[ 4 ] = tr ( "Normal/Boxed" , "Letterform");
		mapModel[ 5 ] = tr ( "Normal/Flattened", "Letterform" ) ;
		mapModel[ 6 ] = tr ( "Normal/Rounded" , "Letterform") ;
		mapModel[ 7 ] = tr ( "Normal/Off Center" , "Letterform") ;
		mapModel[ 8 ] = tr ( "Normal/Square" , "Letterform") ;
		mapModel[ 9 ] = tr ( "Oblique/Contact" , "Letterform") ;
		mapModel[ 10 ] = tr ( "Oblique/Weighted" , "Letterform") ;
		mapModel[ 11 ] = tr ( "Oblique/Boxed" , "Letterform") ;
		mapModel[ 12 ] = tr ( "Oblique/Flattened" , "Letterform") ;
		mapModel[ 13 ] = tr ( "Oblique/Rounded" , "Letterform") ;
		mapModel[ 14 ] = tr ( "Oblique/Off Center" , "Letterform") ;
		mapModel[ 15 ] = tr ( "Oblique/Square" , "Letterform") ;

		m_panoseMap[Letterform] = mapModel;
		m_panoseKeyName[Letterform] = tr ( "Letterform" );
		m_panoseKeyInfo[Letterform] ="<h1>Letterform</h1>Roundness and predominant skewing is classified in the Letterform category.";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Standard/Trimmed" ,"Midline") ;
		mapModel[ 3 ] = tr ( "Standard/Pointed" ,"Midline") ;
		mapModel[ 4 ] = tr ( "Standard/Serifed","Midline" ) ;
		mapModel[ 5 ] = tr ( "High/Trimmed" ,"Midline") ;
		mapModel[ 6 ] = tr ( "High/Pointed" ,"Midline") ;
		mapModel[ 7 ] = tr ( "High/Serifed","Midline" ) ;
		mapModel[ 8 ] = tr ( "Constant/Trimmed" ,"Midline") ;
		mapModel[ 9 ] = tr ( "Constant/Pointed" ,"Midline") ;
		mapModel[ 10 ] = tr ( "Constant/Serifed" ,"Midline") ;
		mapModel[ 11 ] = tr ( "Low/Trimmed" ,"Midline") ;
		mapModel[ 12 ] = tr ( "Low/Pointed" ,"Midline") ;
		mapModel[ 13 ] = tr ( "Low/Serifed" ,"Midline") ;

		m_panoseMap[Midline ] = mapModel;
		m_panoseKeyName[Midline] = tr ( "Midline" );
		m_panoseKeyInfo[Midline] ="<h1>Midline</h1>The ninth category in the PANOSE classification system analyzes two traits, the placement of the midline across the uppercase characters and the treatment of diagonal stem apexes.";
		mapModel.clear();

		mapModel[ 0 ] = tr ( "Any" ) ;
		mapModel[ 1 ] = tr ( "No Fit" ) ;
		mapModel[ 2 ] = tr ( "Constant/Small" , "X-Height") ;
		mapModel[ 3 ] = tr ( "Constant/Standard" , "X-Height") ;
		mapModel[ 4 ] = tr ( "Constant/Large" , "X-Height") ;
		mapModel[ 5 ] = tr ( "Ducking/Small" , "X-Height") ;
		mapModel[ 6 ] = tr ( "Ducking/Standard" , "X-Height") ;
		mapModel[ 7 ] = tr ( "Ducking/Large" , "X-Height") ;

		m_panoseMap[XHeight ] = mapModel;
		m_panoseKeyName[XHeight] = tr ( "X-Height" );
		m_panoseKeyInfo[XHeight] ="<h1>X-Height</h1>Two different traits are represented in the X-height digit: the treatment of uppercase glyphs with diacritical marks and the relative size of the lowercase characters.";

}

void FontStrings::panoseFromFile(const QString & path)
{
	return;
}
void FontStrings::fillCharsetMap()
{
	charsetMap[FT_ENCODING_NONE] = "None";
	charsetMap[FT_ENCODING_UNICODE] = "Unicode";
	charsetMap[FT_ENCODING_MS_SYMBOL] = "MS Symbol";
	charsetMap[FT_ENCODING_SJIS] = "SJIS";
	charsetMap[FT_ENCODING_GB2312	] = "GB2312";
	charsetMap[FT_ENCODING_BIG5] = "BIG5";
	charsetMap[FT_ENCODING_WANSUNG] = "Wansung";
	charsetMap[FT_ENCODING_JOHAB] = "Johab";
	charsetMap[FT_ENCODING_ADOBE_LATIN_1] = "Adobe Latin 1";
	charsetMap[FT_ENCODING_ADOBE_STANDARD] = "Adobe Standard";
	charsetMap[FT_ENCODING_ADOBE_EXPERT] = "Adobe Expert";
	charsetMap[FT_ENCODING_ADOBE_CUSTOM] = "Adobe Custom";
	charsetMap[FT_ENCODING_APPLE_ROMAN] = "Apple Roman";
	charsetMap[FT_ENCODING_OLD_LATIN_2] = tr ( "This value is deprecated and was never used nor reported by FreeType. Don't use or test for it." );
	charsetMap[FT_ENCODING_MS_SJIS] = "MS SJIS";
	charsetMap[FT_ENCODING_MS_GB2312] = "MS GB2312";
	charsetMap[FT_ENCODING_MS_BIG5] = "MS BIG5";
	charsetMap[FT_ENCODING_MS_WANSUNG] = "MS Wansung";
	charsetMap[FT_ENCODING_MS_JOHAB] = "MS Johab";
}

void FontStrings::fillTTTableList()
{
	tttableList.clear();
	
	// Required Tables
	tttableList["cmap"] = tr("Character to glyph mapping");
	tttableList["head"] = tr("Font header");
	tttableList["hhea"] = tr("Horizontal header");
	tttableList["hmtx"] = tr("Horizontal metrics");
	tttableList["maxp"] = tr("Maximum profile");
	tttableList["name"] = tr("Naming table");
	tttableList["OS/2"] = tr("OS/2 and Windows specific metrics");
	tttableList["post"] = tr("PostScript information");
	
	// Tables Related to TrueType Outlines
	tttableList["cvt"] = tr("Control Value Table");
	tttableList["fpgm"] = tr("Font program");
	tttableList["glyf"] = tr("Glyph data");
	tttableList["loca"] = tr("Index to location");
	tttableList["prep"] = tr("CVT Program");
	
	// Tables Related to PostScript Outlines
	tttableList["CFF"] = tr("PostScript font program");
	tttableList["VORG"] = tr("Vertical Origin");
	
	// Tables Related to Bitmap Glyphs
	tttableList["EBDT"] = tr("Embedded bitmap data");
	tttableList["EBLC"] = tr("Embedded bitmap location data");
	tttableList["EBSC"] = tr("Embedded bitmap scaling data");
	
	// Advanced Typographic Tables
	tttableList["BASE"] = tr("Baseline data");
	tttableList["GDEF"] = tr("Glyph definition data");
	tttableList["GPOS"] = tr("Glyph positioning data");
	tttableList["GSUB"] = tr("Glyph substitution data");
	tttableList["JSTF"] = tr("Justification data");
	
	// Other OpenType Tables
	tttableList["DSIG"] = tr("Digital signature");
	tttableList["gasp"] = tr("Grid-fitting/Scan-conversion");
	tttableList["hdmx"] = tr("Horizontal device metrics");
	tttableList["kern"] = tr("Kerning");
	tttableList["LTSH"] = tr("Linear threshold data");
	tttableList["PCLT"] = tr("PCL 5 data");
	tttableList["VDMX"] = tr("Vertical device metrics");
	tttableList["vhea"] = tr("Vertical Metrics header");
	tttableList["vmtx"] = tr("Vertical Metrics");
}


void FontStrings::fillFSftypeMap()
{
	// From http://www.microsoft.com/typography/otspec/os2.htm#fst
	
	m_FsType[FontItem::NOT_RESTRICTED] = tr("This font may be embedded and permanently installed on the remote system by an application. The user of the remote system acquires the identical rights, obligations and licenses for that font as the original purchaser of the font, and is subject to the same end-user license agreement, copyright, design patent, and/or trademark as was the original purchaser.");
	m_FsType[FontItem::RESTRICTED] = tr("This font must not be modified, embedded or exchanged in any manner without first obtaining permission of the legal owner.");
	m_FsType[FontItem::PREVIEW_PRINT] = tr("This font may be embedded, and temporarily loaded on the remote system. Documents containing this font must be opened \"read-only;\" no edits can be applied to the document.");
	m_FsType[FontItem::EDIT_EMBED] = tr("This font may be embedded but must only be installed  temporarily  on other systems. In contrast to Preview &amp; Print fonts, documents containing this font may be opened for reading, editing is permitted, and changes may be saved.");
	m_FsType[FontItem::NOSUBSET] = tr("This font may not be subsetted prior to embedding. Other embedding restrictions specified in bits 0-3 and 9 also apply.");
	m_FsType[FontItem::BITMAP_ONLY] = tr("Only bitmaps contained in this font may be embedded. No outline data may be embedded. If there are no bitmaps available in this font, then it is considered unembeddable and the embedding services will fail. Other embedding restrictions specified in bits 0-3 and 8 also apply.");
}


const QMap< FMFontDb::InfoItem, QString >& FontStrings::Names()
{
	FontStrings *that(getInstance());
	return that->m_name;
}

const QMap< FontStrings::PanoseKey, QMap < int , QString > >& FontStrings::Panose()
{
	FontStrings *that(getInstance());
	return that->m_panoseMap;
}

const QString FontStrings::PanoseKeyName(PanoseKey pk)
{
	FontStrings *that(getInstance());
	return that->m_panoseKeyName.value(pk);
}

const QString FontStrings::PanoseKeyInfo(PanoseKey pk)
{
	FontStrings *that(getInstance());
	return that->m_panoseKeyInfo.value(pk);
}

const QString FontStrings::Encoding(FT_Encoding enc)
{
	FontStrings *that(getInstance());
	return that->charsetMap.value(enc);
}

const QMap< QString, QString > & FontStrings::Tables()
{
	FontStrings *that(getInstance());
	return that->tttableList;
}

QString FontStrings::FsType(int fstype_part, bool shortString)
{
	FontStrings *that(getInstance());
	if(!shortString)
		return that->m_FsType[fstype_part];
	else
	{
		if(FontItem::NOT_RESTRICTED == fstype_part)
			return tr("Not Restricted");
		else if(FontItem::RESTRICTED == fstype_part)
			return tr("Restricted");
		else if(FontItem::PREVIEW_PRINT == fstype_part)
			return tr("Preview/Print");
		else if(FontItem::EDIT_EMBED == fstype_part)
			return tr("Edit/Embed");
		else if(FontItem::NOSUBSET == fstype_part)
			return tr("No Subset");
		else if(FontItem::BITMAP_ONLY == fstype_part)
			return tr("Bitmap Only");
	}

	return QString();
}

FontStrings::PanoseKey FontStrings::firstPanoseKey()
{
	return FamilyType;
}

FontStrings::PanoseKey FontStrings::nextPanoseKey ( PanoseKey pk )
{
	switch ( pk )
	{
		case FamilyType:return SerifStyle;break;
		case SerifStyle:return Weight;break;
		case Weight:return Proportion;break;
		case Proportion:return Contrast;break;
		case Contrast:return StrokeVariation;break;
		case StrokeVariation:return ArmStyle;break;
		case ArmStyle:return Letterform;break;
		case Letterform:return Midline;break;
		case Midline:return XHeight;break;
		default:return InvalidPK;
	}
	return InvalidPK;
}



