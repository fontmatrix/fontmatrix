/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmfontstrings.h"
#include "fmpaths.h"

#include <KLocalizedString>
#include <QFile>

FontStrings *FontStrings::instance = nullptr;
FontStrings::FontStrings()
{
    fillNamesMeaning();
    fillPanoseMap();
    fillCharsetMap();
    fillTTTableList();
    fillFSftypeMap();
    fillUniCat();
    fillScriptTagNames();
}

FontStrings *FontStrings::getInstance()
{
    if (!instance)
        instance = new FontStrings;
    return instance;
}

void FontStrings::fillNamesMeaning()
{
    m_name[FMFontDb::Copyright] = i18nc("@label font information field", "Copyright");
    m_name[FMFontDb::FontFamily] = i18nc("@label font information field", "Font Family");
    m_name[FMFontDb::FontSubfamily] = i18nc("@label font information field", "Font Subfamily");
    m_name[FMFontDb::UniqueFontIdentifier] = i18nc("@label font information field", "Unique font identifier");
    m_name[FMFontDb::FullFontName] = i18nc("@label font information field", "Full font name");
    m_name[FMFontDb::VersionString] = i18nc("@label font information field", "Version string");
    m_name[FMFontDb::PostscriptName] = i18nc("@label font information field", "Postscript name");
    m_name[FMFontDb::Trademark] = i18nc("@label font information field", "Trademark");
    m_name[FMFontDb::ManufacturerName] = i18nc("@label font information field", "Manufacturer");
    m_name[FMFontDb::Designer] = i18nc("@label font information field", "Designer");
    m_name[FMFontDb::Description] = i18nc("@label font information field", "Description");
    m_name[FMFontDb::URLVendor] = i18nc("@label font information field", "URL Vendor");
    m_name[FMFontDb::URLDesigner] = i18nc("@label font information field", "URL Designer");
    m_name[FMFontDb::LicenseDescription] = i18nc("@label font information field", "License Description");
    m_name[FMFontDb::LicenseInfoURL] = i18nc("@label font information field", "License Info URL");
    m_name[FMFontDb::PreferredFamily] = i18nc("@label font information field", "Preferred Family");
    m_name[FMFontDb::PreferredSubfamily] = i18nc("@label font information field", "Preferred Subfamily");
    m_name[FMFontDb::CompatibleMacintosh] = i18nc("@label font information field", "Compatible Full (Macintosh only)");
    m_name[FMFontDb::SampleText] = i18nc("@label font information field", "Sample text");
    m_name[FMFontDb::PostScriptCIDName] = i18nc("@label font information field", "PostScript CID findfont name");
    // 	m_name[FMFontDb::Panose]= i18n("Panose");
    m_name[FMFontDb::AllInfo] = i18nc("@label font information field", "All fields");
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
    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Family Type", "Text and Display");
    mapModel[3] = i18nc("Family Type", "Script");
    mapModel[4] = i18nc("Family Type", "Decorative");
    mapModel[5] = i18nc("Family Type", "Pictorial");

    m_panoseMap[FamilyType] = mapModel;
    m_panoseKeyName[FamilyType] = i18nc("@label PANOSE classification digit", "Family Type");
    m_panoseKeyInfo[FamilyType] = "<h1>Family Kind</h1> Defines what type of font is being classified.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Serif style", "Cove");
    mapModel[3] = i18nc("Serif style", "Obtuse Cove");
    mapModel[4] = i18nc("Serif style", "Square Cove");
    mapModel[5] = i18nc("Serif style", "Obtuse Square Cove");
    mapModel[6] = i18nc("Serif style", "Square");
    mapModel[7] = i18nc("Serif style", "Thin");
    mapModel[8] = i18nc("Serif style", "Bone");
    mapModel[9] = i18nc("Serif style", "Exaggerated");
    mapModel[10] = i18nc("Serif style", "Triangle");
    mapModel[11] = i18nc("Serif style", "Normal Sans");
    mapModel[12] = i18nc("Serif style", "Obtuse Sans");
    mapModel[13] = i18nc("Serif style", "Perp Sans");
    mapModel[14] = i18nc("Serif style", "Flared");
    mapModel[15] = i18nc("Serif style", "Rounded");

    m_panoseMap[SerifStyle] = mapModel;
    m_panoseKeyName[SerifStyle] = i18nc("@label PANOSE classification digit", "Serif style");
    m_panoseKeyInfo[SerifStyle] = "<h1>Serif style</h1>This digit describes the appearance of the serifs used in a font design";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Weight", "Very Light");
    mapModel[3] = i18nc("Weight", "Light");
    mapModel[4] = i18nc("Weight", "Thin");
    mapModel[5] = i18nc("Weight", "Book");
    mapModel[6] = i18nc("Weight", "Medium");
    mapModel[7] = i18nc("Weight", "Demi");
    mapModel[8] = i18nc("Weight", "Bold");
    mapModel[9] = i18nc("Weight", "Heavy");
    mapModel[10] = i18nc("Weight", "Black");
    mapModel[11] = i18nc("Weight", "Nord");

    m_panoseMap[Weight] = mapModel;
    m_panoseKeyName[Weight] = i18nc("@label PANOSE classification digit", "Weight");
    m_panoseKeyInfo[Weight] = "<h1>Weight</h1>The Weight digit classifies the appearance of a fonts’ stroke thickness in relation to its height.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Proportion", "Old Style");
    mapModel[3] = i18nc("Proportion", "Modern");
    mapModel[4] = i18nc("Proportion", "Even Width");
    mapModel[5] = i18nc("Proportion", "Expanded");
    mapModel[6] = i18nc("Proportion", "Condensed");
    mapModel[7] = i18nc("Proportion", "Very Expanded");
    mapModel[8] = i18nc("Proportion", "Very Condensed");
    mapModel[9] = i18nc("Proportion", "Monospaced");

    m_panoseMap[Proportion] = mapModel;
    m_panoseKeyName[Proportion] = i18nc("@label PANOSE classification digit", "Proportion");
    m_panoseKeyInfo[Proportion] =
        "<h1>Proportion</h1>The proportion of a font in the PANOSE Typeface Matching System is defined in greater detail than simply an indication of general "
        "glyph shape aspect ratio such as extended and condensed.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Contrast", "None");
    mapModel[3] = i18nc("Contrast", "Very Low");
    mapModel[4] = i18nc("Contrast", "Low");
    mapModel[5] = i18nc("Contrast", "Medium Low");
    mapModel[6] = i18nc("Contrast", "Medium");
    mapModel[7] = i18nc("Contrast", "Medium High");
    mapModel[8] = i18nc("Contrast", "High");
    mapModel[9] = i18nc("Contrast", "Very High");

    m_panoseMap[Contrast] = mapModel;
    m_panoseKeyName[Contrast] = i18nc("@label PANOSE classification digit", "Contrast");
    m_panoseKeyInfo[Contrast] =
        "<h1>Contrast</h1>The Contrast digit describes the ratio between the thickest point on the stroke of the letter O and the narrowest point on the "
        "letter O.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Stroke Variation", "Gradual/Diagonal");
    mapModel[3] = i18nc("Stroke Variation", "Gradual/Transitional");
    mapModel[4] = i18nc("Stroke Variation", "Gradual/Vertical");
    mapModel[5] = i18nc("Stroke Variation", "Gradual/Horizontal");
    mapModel[6] = i18nc("Stroke Variation", "Rapid/Vertical");
    mapModel[7] = i18nc("Stroke Variation", "Rapid/Horizontal");
    mapModel[8] = i18nc("Stroke Variation", "Instant/Vertical");

    m_panoseMap[StrokeVariation] = mapModel;
    m_panoseKeyName[StrokeVariation] = i18nc("@label PANOSE classification digit", "Stroke Variation");
    m_panoseKeyInfo[StrokeVariation] =
        "<h1>Stroke Variation</h1>The Stroke Variation category further details the contrast trait by describing the kind of transition that occurs as the "
        "stem thickness changes on rounded glyph shapes.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Arm Style", "Straight Arms/Horizontal");
    mapModel[3] = i18nc("Arm Style", "Straight Arms/Wedge");
    mapModel[4] = i18nc("Arm Style", "Straight Arms/Vertical");
    mapModel[5] = i18nc("Arm Style", "Straight Arms/Single Serif");
    mapModel[6] = i18nc("Arm Style", "Straight Arms/Double Serif");
    mapModel[7] = i18nc("Arm Style", "Non-Straight Arms/Horizontal");
    mapModel[8] = i18nc("Arm Style", "Non-Straight Arms/Wedge");
    mapModel[9] = i18nc("Arm Style", "Non-Straight Arms/Vertical");
    mapModel[10] = i18nc("Arm Style", "Non-Straight Arms/Single Serif");
    mapModel[11] = i18nc("Arm Style", "Non-Straight Arms/Double Serif");

    m_panoseMap[ArmStyle] = mapModel;
    m_panoseKeyName[ArmStyle] = i18nc("@label PANOSE classification digit", "Arm Style");
    m_panoseKeyInfo[ArmStyle] =
        "<h1>Arm Style</h1>The Arm Style category classifies two attributes of a glyph design: special treatment of diagonal stems and termination of open "
        "rounded letterforms.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Letterform", "Normal/Contact");
    mapModel[3] = i18nc("Letterform", "Normal/Weighted");
    mapModel[4] = i18nc("Letterform", "Normal/Boxed");
    mapModel[5] = i18nc("Letterform", "Normal/Flattened");
    mapModel[6] = i18nc("Letterform", "Normal/Rounded");
    mapModel[7] = i18nc("Letterform", "Normal/Off Center");
    mapModel[8] = i18nc("Letterform", "Normal/Square");
    mapModel[9] = i18nc("Letterform", "Oblique/Contact");
    mapModel[10] = i18nc("Letterform", "Oblique/Weighted");
    mapModel[11] = i18nc("Letterform", "Oblique/Boxed");
    mapModel[12] = i18nc("Letterform", "Oblique/Flattened");
    mapModel[13] = i18nc("Letterform", "Oblique/Rounded");
    mapModel[14] = i18nc("Letterform", "Oblique/Off Center");
    mapModel[15] = i18nc("Letterform", "Oblique/Square");

    m_panoseMap[Letterform] = mapModel;
    m_panoseKeyName[Letterform] = i18nc("@label PANOSE classification digit", "Letterform");
    m_panoseKeyInfo[Letterform] = "<h1>Letterform</h1>Roundness and predominant skewing is classified in the Letterform category.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("Midline", "Standard/Trimmed");
    mapModel[3] = i18nc("Midline", "Standard/Pointed");
    mapModel[4] = i18nc("Midline", "Standard/Serifed");
    mapModel[5] = i18nc("Midline", "High/Trimmed");
    mapModel[6] = i18nc("Midline", "High/Pointed");
    mapModel[7] = i18nc("Midline", "High/Serifed");
    mapModel[8] = i18nc("Midline", "Constant/Trimmed");
    mapModel[9] = i18nc("Midline", "Constant/Pointed");
    mapModel[10] = i18nc("Midline", "Constant/Serifed");
    mapModel[11] = i18nc("Midline", "Low/Trimmed");
    mapModel[12] = i18nc("Midline", "Low/Pointed");
    mapModel[13] = i18nc("Midline", "Low/Serifed");

    m_panoseMap[Midline] = mapModel;
    m_panoseKeyName[Midline] = i18nc("@label PANOSE classification digit", "Midline");
    m_panoseKeyInfo[Midline] =
        "<h1>Midline</h1>The ninth category in the PANOSE classification system analyzes two traits, the placement of the midline across the uppercase "
        "characters and the treatment of diagonal stem apexes.";
    mapModel.clear();

    mapModel[0] = i18nc("@item:inlistbox PANOSE value", "Any");
    mapModel[1] = i18nc("@item:inlistbox PANOSE value", "No Fit");
    mapModel[2] = i18nc("X-Height", "Constant/Small");
    mapModel[3] = i18nc("X-Height", "Constant/Standard");
    mapModel[4] = i18nc("X-Height", "Constant/Large");
    mapModel[5] = i18nc("X-Height", "Ducking/Small");
    mapModel[6] = i18nc("X-Height", "Ducking/Standard");
    mapModel[7] = i18nc("X-Height", "Ducking/Large");

    m_panoseMap[XHeight] = mapModel;
    m_panoseKeyName[XHeight] = i18nc("@label PANOSE classification digit", "X-Height");
    m_panoseKeyInfo[XHeight] =
        "<h1>X-Height</h1>Two different traits are represented in the X-height digit: the treatment of uppercase glyphs with diacritical marks and the "
        "relative size of the lowercase characters.";
}

void FontStrings::panoseFromFile(const QString &)
{
    return;
}
void FontStrings::fillCharsetMap()
{
    charsetMap[FT_ENCODING_NONE] = "None";
    charsetMap[FT_ENCODING_UNICODE] = "Unicode";
    charsetMap[FT_ENCODING_MS_SYMBOL] = "MS Symbol";
    charsetMap[FT_ENCODING_SJIS] = "SJIS";
    charsetMap[FT_ENCODING_GB2312] = "GB2312";
    charsetMap[FT_ENCODING_BIG5] = "BIG5";
    charsetMap[FT_ENCODING_WANSUNG] = "Wansung";
    charsetMap[FT_ENCODING_JOHAB] = "Johab";
    charsetMap[FT_ENCODING_ADOBE_LATIN_1] = "Adobe Latin 1";
    charsetMap[FT_ENCODING_ADOBE_STANDARD] = "Adobe Standard";
    charsetMap[FT_ENCODING_ADOBE_EXPERT] = "Adobe Expert";
    charsetMap[FT_ENCODING_ADOBE_CUSTOM] = "Adobe Custom";
    charsetMap[FT_ENCODING_APPLE_ROMAN] = "Apple Roman";
    charsetMap[FT_ENCODING_OLD_LATIN_2] = i18nc("@info", "This value is deprecated and was never used nor reported by FreeType. Don't use or test for it.");
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
    tttableList["cmap"] = i18nc("@item:intable TrueType table", "Character to glyph mapping");
    tttableList["head"] = i18nc("@item:intable TrueType table", "Font header");
    tttableList["hhea"] = i18nc("@item:intable TrueType table", "Horizontal header");
    tttableList["hmtx"] = i18nc("@item:intable TrueType table", "Horizontal metrics");
    tttableList["maxp"] = i18nc("@item:intable TrueType table", "Maximum profile");
    tttableList["name"] = i18nc("@item:intable TrueType table", "Naming table");
    tttableList["OS/2"] = i18nc("@item:intable TrueType table", "OS/2 and Windows specific metrics");
    tttableList["post"] = i18nc("@item:intable TrueType table", "PostScript information");

    // Tables Related to TrueType Outlines
    tttableList["cvt"] = i18nc("@item:intable TrueType table", "Control Value Table");
    tttableList["fpgm"] = i18nc("@item:intable TrueType table", "Font program");
    tttableList["glyf"] = i18nc("@item:intable TrueType table", "Glyph data");
    tttableList["loca"] = i18nc("@item:intable TrueType table", "Index to location");
    tttableList["prep"] = i18nc("@item:intable TrueType table", "CVT Program");

    // Tables Related to PostScript Outlines
    tttableList["CFF"] = i18nc("@item:intable TrueType table", "PostScript font program");
    tttableList["VORG"] = i18nc("@item:intable TrueType table", "Vertical Origin");

    // Tables Related to Bitmap Glyphs
    tttableList["EBDT"] = i18nc("@item:intable TrueType table", "Embedded bitmap data");
    tttableList["EBLC"] = i18nc("@item:intable TrueType table", "Embedded bitmap location data");
    tttableList["EBSC"] = i18nc("@item:intable TrueType table", "Embedded bitmap scaling data");

    // Advanced Typographic Tables
    tttableList["BASE"] = i18nc("@item:intable TrueType table", "Baseline data");
    tttableList["GDEF"] = i18nc("@item:intable TrueType table", "Glyph definition data");
    tttableList["GPOS"] = i18nc("@item:intable TrueType table", "Glyph positioning data");
    tttableList["GSUB"] = i18nc("@item:intable TrueType table", "Glyph substitution data");
    tttableList["JSTF"] = i18nc("@item:intable TrueType table", "Justification data");

    // Other OpenType Tables
    tttableList["DSIG"] = i18nc("@item:intable TrueType table", "Digital signature");
    tttableList["gasp"] = i18nc("@item:intable TrueType table", "Grid-fitting/Scan-conversion");
    tttableList["hdmx"] = i18nc("@item:intable TrueType table", "Horizontal device metrics");
    tttableList["kern"] = i18nc("@item:intable TrueType table", "Kerning");
    tttableList["LTSH"] = i18nc("@item:intable TrueType table", "Linear threshold data");
    tttableList["PCLT"] = i18nc("@item:intable TrueType table", "PCL 5 data");
    tttableList["VDMX"] = i18nc("@item:intable TrueType table", "Vertical device metrics");
    tttableList["vhea"] = i18nc("@item:intable TrueType table", "Vertical Metrics header");
    tttableList["vmtx"] = i18nc("@item:intable TrueType table", "Vertical Metrics");
}

void FontStrings::fillFSftypeMap()
{
    // From http://www.microsoft.com/typography/otspec/os2.htm#fst

    m_FsType[FontItem::NOT_RESTRICTED] = i18nc(
        "@info font embedding permission",
        "This font may be embedded and permanently installed on the remote system by an application. The user of the remote system acquires the identical "
        "rights, obligations and licenses for that font as the original purchaser of the font, and is subject to the same end-user license agreement, "
        "copyright, design patent, and/or trademark as was the original purchaser.");
    m_FsType[FontItem::RESTRICTED] =
        i18nc("@info font embedding permission",
              "This font must not be modified, embedded or exchanged in any manner without first obtaining permission of the legal owner.");
    m_FsType[FontItem::PREVIEW_PRINT] = i18nc(
        "@info font embedding permission",
        "This font may be embedded, and temporarily loaded on the remote system. Documents containing this font must be opened \"read-only;\" no edits can be "
        "applied to the document.");
    m_FsType[FontItem::EDIT_EMBED] = i18nc(
        "@info font embedding permission",
        "This font may be embedded but must only be installed  temporarily  on other systems. In contrast to Preview &amp; Print fonts, documents containing "
        "this font may be opened for reading, editing is permitted, and changes may be saved.");
    m_FsType[FontItem::NOSUBSET] =
        i18nc("@info font embedding permission",
              "This font may not be subsetted prior to embedding. Other embedding restrictions specified in bits 0-3 and 9 also apply.");
    m_FsType[FontItem::BITMAP_ONLY] = i18nc(
        "@info font embedding permission",
        "Only bitmaps contained in this font may be embedded. No outline data may be embedded. If there are no bitmaps available in this font, then it is "
        "considered unembeddable and the embedding services will fail. Other embedding restrictions specified in bits 0-3 and 8 also apply.");
}

void FontStrings::fillUniCat()
{
    unicodeCategory[QChar::Mark_NonSpacing] = i18nc("@item:intable Unicode general category", "Mark, NonSpacing");
    unicodeCategory[QChar::Mark_SpacingCombining] = i18nc("@item:intable Unicode general category", "Mark, SpacingCombining");
    unicodeCategory[QChar::Mark_Enclosing] = i18nc("@item:intable Unicode general category", "Mark, Enclosing");
    unicodeCategory[QChar::Number_DecimalDigit] = i18nc("@item:intable Unicode general category", "Number, DecimalDigit");
    unicodeCategory[QChar::Number_Letter] = i18nc("@item:intable Unicode general category", "Number, Letter");
    unicodeCategory[QChar::Number_Other] = i18nc("@item:intable Unicode general category", "Number, Other");
    unicodeCategory[QChar::Separator_Space] = i18nc("@item:intable Unicode general category", "Separator, Space");
    unicodeCategory[QChar::Separator_Line] = i18nc("@item:intable Unicode general category", "Separator, Line");
    unicodeCategory[QChar::Separator_Paragraph] = i18nc("@item:intable Unicode general category", "Separator, Paragraph");
    unicodeCategory[QChar::Other_Control] = i18nc("@item:intable Unicode general category", "Other, Control");
    unicodeCategory[QChar::Other_Format] = i18nc("@item:intable Unicode general category", "Other, Format");
    unicodeCategory[QChar::Other_Surrogate] = i18nc("@item:intable Unicode general category", "Other, Surrogate");
    unicodeCategory[QChar::Other_PrivateUse] = i18nc("@item:intable Unicode general category", "Other, PrivateUse");
    unicodeCategory[QChar::Other_NotAssigned] = i18nc("@item:intable Unicode general category", "Other, NotAssigned");
    unicodeCategory[QChar::Letter_Uppercase] = i18nc("@item:intable Unicode general category", "Letter, Uppercase");
    unicodeCategory[QChar::Letter_Lowercase] = i18nc("@item:intable Unicode general category", "Letter, Lowercase");
    unicodeCategory[QChar::Letter_Titlecase] = i18nc("@item:intable Unicode general category", "Letter, Titlecase");
    unicodeCategory[QChar::Letter_Modifier] = i18nc("@item:intable Unicode general category", "Letter, Modifier");
    unicodeCategory[QChar::Letter_Other] = i18nc("@item:intable Unicode general category", "Letter, Other");
    unicodeCategory[QChar::Punctuation_Connector] = i18nc("@item:intable Unicode general category", "Punctuation, Connector");
    unicodeCategory[QChar::Punctuation_Dash] = i18nc("@item:intable Unicode general category", "Punctuation, Dash");
    unicodeCategory[QChar::Punctuation_Open] = i18nc("@item:intable Unicode general category", "Punctuation, Open");
    unicodeCategory[QChar::Punctuation_Close] = i18nc("@item:intable Unicode general category", "Punctuation, Close");
    unicodeCategory[QChar::Punctuation_InitialQuote] = i18nc("@item:intable Unicode general category", "Punctuation, InitialQuote");
    unicodeCategory[QChar::Punctuation_FinalQuote] = i18nc("@item:intable Unicode general category", "Punctuation, FinalQuote");
    unicodeCategory[QChar::Punctuation_Other] = i18nc("@item:intable Unicode general category", "Punctuation, Other");
    unicodeCategory[QChar::Symbol_Math] = i18nc("@item:intable Unicode general category", "Symbol, Math");
    unicodeCategory[QChar::Symbol_Currency] = i18nc("@item:intable Unicode general category", "Symbol, Currency");
    unicodeCategory[QChar::Symbol_Modifier] = i18nc("@item:intable Unicode general category", "Symbol, Modifier");
    unicodeCategory[QChar::Symbol_Other] = i18nc("@item:intable Unicode general category", "Symbol, Other");
}

void FontStrings::fillScriptTagNames()
{
    scriptTagNames[QString("arab")] = i18nc("@item:inlistbox OpenType script", "Arabic");
    scriptTagNames[QString("armn")] = i18nc("@item:inlistbox OpenType script", "Armenian");
    scriptTagNames[QString("bali")] = i18nc("@item:inlistbox OpenType script", "Balinese");
    scriptTagNames[QString("beng")] = i18nc("@item:inlistbox OpenType script", "Bengali");
    scriptTagNames[QString("bopo")] = i18nc("@item:inlistbox OpenType script", "Bopomofo");
    scriptTagNames[QString("brai")] = i18nc("@item:inlistbox OpenType script", "Braille");
    scriptTagNames[QString("bugi")] = i18nc("@item:inlistbox OpenType script", "Buginese");
    scriptTagNames[QString("buhd")] = i18nc("@item:inlistbox OpenType script", "Buhid");
    scriptTagNames[QString("byzm")] = i18nc("@item:inlistbox OpenType script", "Byzantine Music");
    scriptTagNames[QString("cans")] = i18nc("@item:inlistbox OpenType script", "Canadian Syllabics");
    scriptTagNames[QString("cher")] = i18nc("@item:inlistbox OpenType script", "Cherokee");
    scriptTagNames[QString("hani")] = i18nc("@item:inlistbox OpenType script", "CJK Ideographic");
    scriptTagNames[QString("copt")] = i18nc("@item:inlistbox OpenType script", "Coptic");
    scriptTagNames[QString("cprt")] = i18nc("@item:inlistbox OpenType script", "Cypriot Syllabary");
    scriptTagNames[QString("cyrl")] = i18nc("@item:inlistbox OpenType script", "Cyrillic");
    scriptTagNames[QString("DFLT")] = i18nc("@item:inlistbox OpenType script", "Default");
    scriptTagNames[QString("dsrt")] = i18nc("@item:inlistbox OpenType script", "Deseret");
    scriptTagNames[QString("deva")] = i18nc("@item:inlistbox OpenType script", "Devanagari");
    scriptTagNames[QString("ethi")] = i18nc("@item:inlistbox OpenType script", "Ethiopic");
    scriptTagNames[QString("geor")] = i18nc("@item:inlistbox OpenType script", "Georgian");
    scriptTagNames[QString("glag")] = i18nc("@item:inlistbox OpenType script", "Glagolitic");
    scriptTagNames[QString("goth")] = i18nc("@item:inlistbox OpenType script", "Gothic");
    scriptTagNames[QString("grek")] = i18nc("@item:inlistbox OpenType script", "Greek");
    scriptTagNames[QString("gujr")] = i18nc("@item:inlistbox OpenType script", "Gujarati");
    scriptTagNames[QString("guru")] = i18nc("@item:inlistbox OpenType script", "Gurmukhi");
    scriptTagNames[QString("jamo")] = i18nc("@item:inlistbox OpenType script", "Hangul Jamo");
    scriptTagNames[QString("hang")] = i18nc("@item:inlistbox OpenType script", "Hangul");
    scriptTagNames[QString("hano")] = i18nc("@item:inlistbox OpenType script", "Hanunoo");
    scriptTagNames[QString("hebr")] = i18nc("@item:inlistbox OpenType script", "Hebrew");
    scriptTagNames[QString("kana")] = i18nc("@item:inlistbox OpenType script", "Hiragana and Katakana");
    scriptTagNames[QString("java")] = i18nc("@item:inlistbox OpenType script", "Javanese");
    scriptTagNames[QString("knda")] = i18nc("@item:inlistbox OpenType script", "Kannada");
    scriptTagNames[QString("khar")] = i18nc("@item:inlistbox OpenType script", "Kharosthi");
    scriptTagNames[QString("khmr")] = i18nc("@item:inlistbox OpenType script", "Khmer");
    scriptTagNames[QString("lao ")] = i18nc("@item:inlistbox OpenType script", "Lao");
    scriptTagNames[QString("latn")] = i18nc("@item:inlistbox OpenType script", "Latin");
    scriptTagNames[QString("limb")] = i18nc("@item:inlistbox OpenType script", "Limbu");
    scriptTagNames[QString("linb")] = i18nc("@item:inlistbox OpenType script", "Linear B");
    scriptTagNames[QString("mlym")] = i18nc("@item:inlistbox OpenType script", "Malayalam");
    scriptTagNames[QString("math")] = i18nc("@item:inlistbox OpenType script", "Mathematical Alphanumeric Symbols");
    scriptTagNames[QString("mong")] = i18nc("@item:inlistbox OpenType script", "Mongolian");
    scriptTagNames[QString("musc")] = i18nc("@item:inlistbox OpenType script", "Musical Symbols");
    scriptTagNames[QString("mymr")] = i18nc("@item:inlistbox OpenType script", "Myanmar");
    scriptTagNames[QString("nko ")] = i18nc("@item:inlistbox OpenType script", "N'ko");
    scriptTagNames[QString("ogam")] = i18nc("@item:inlistbox OpenType script", "Ogham");
    scriptTagNames[QString("ital")] = i18nc("@item:inlistbox OpenType script", "Old Italic");
    scriptTagNames[QString("xpeo")] = i18nc("@item:inlistbox OpenType script", "Old Persian Cuneiform");
    scriptTagNames[QString("orya")] = i18nc("@item:inlistbox OpenType script", "Oriya");
    scriptTagNames[QString("osma")] = i18nc("@item:inlistbox OpenType script", "Osmanya");
    scriptTagNames[QString("phag")] = i18nc("@item:inlistbox OpenType script", "Phags-pa");
    scriptTagNames[QString("phnx")] = i18nc("@item:inlistbox OpenType script", "Phoenician");
    scriptTagNames[QString("runr")] = i18nc("@item:inlistbox OpenType script", "Runic");
    scriptTagNames[QString("shaw")] = i18nc("@item:inlistbox OpenType script", "Shavian");
    scriptTagNames[QString("sinh")] = i18nc("@item:inlistbox OpenType script", "Sinhala");
    scriptTagNames[QString("xsux")] = i18nc("@item:inlistbox OpenType script", "Sumero-Akkadian Cuneiform");
    scriptTagNames[QString("sylo")] = i18nc("@item:inlistbox OpenType script", "Syloti Nagri");
    scriptTagNames[QString("syrc")] = i18nc("@item:inlistbox OpenType script", "Syriac");
    scriptTagNames[QString("tglg")] = i18nc("@item:inlistbox OpenType script", "Tagalog");
    scriptTagNames[QString("tagb")] = i18nc("@item:inlistbox OpenType script", "Tagbanwa");
    scriptTagNames[QString("tale")] = i18nc("@item:inlistbox OpenType script", "Tai Le");
    scriptTagNames[QString("talu")] = i18nc("@item:inlistbox OpenType script", "Tai Lu");
    scriptTagNames[QString("taml")] = i18nc("@item:inlistbox OpenType script", "Tamil");
    scriptTagNames[QString("telu")] = i18nc("@item:inlistbox OpenType script", "Telugu");
    scriptTagNames[QString("thaa")] = i18nc("@item:inlistbox OpenType script", "Thaana");
    scriptTagNames[QString("thai")] = i18nc("@item:inlistbox OpenType script", "Thai");
    scriptTagNames[QString("tibt")] = i18nc("@item:inlistbox OpenType script", "Tibetan");
    scriptTagNames[QString("tfng")] = i18nc("@item:inlistbox OpenType script", "Tifinagh");
    scriptTagNames[QString("ugar")] = i18nc("@item:inlistbox OpenType script", "Ugaritic Cuneiform");
    scriptTagNames[QString("yi ")] = i18nc("@item:inlistbox OpenType script", "Yi");
}

const QMap<FMFontDb::InfoItem, QString> &FontStrings::Names()
{
    FontStrings *that(getInstance());
    return that->m_name;
}

const QMap<FontStrings::PanoseKey, QMap<int, QString>> &FontStrings::Panose()
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

const QMap<QString, QString> &FontStrings::Tables()
{
    FontStrings *that(getInstance());
    return that->tttableList;
}

QString FontStrings::FsType(int fstype_part, bool shortString)
{
    FontStrings *that(getInstance());
    if (!shortString)
        return that->m_FsType.value(fstype_part);
    else {
        if (FontItem::NOT_RESTRICTED == fstype_part)
            return i18nc("@label font embedding permission", "Not Restricted");
        else if (FontItem::RESTRICTED == fstype_part)
            return i18nc("@label font embedding permission", "Restricted");
        else if (FontItem::PREVIEW_PRINT == fstype_part)
            return i18nc("@label font embedding permission", "Preview/Print");
        else if (FontItem::EDIT_EMBED == fstype_part)
            return i18nc("@label font embedding permission", "Edit/Embed");
        else if (FontItem::NOSUBSET == fstype_part)
            return i18nc("@label font embedding permission", "No Subset");
        else if (FontItem::BITMAP_ONLY == fstype_part)
            return i18nc("@label font embedding permission", "Bitmap Only");
    }

    return QString();
}

FontStrings::PanoseKey FontStrings::firstPanoseKey()
{
    return FamilyType;
}

FontStrings::PanoseKey FontStrings::nextPanoseKey(PanoseKey pk)
{
    switch (pk) {
    case FamilyType:
        return SerifStyle;
        break;
    case SerifStyle:
        return Weight;
        break;
    case Weight:
        return Proportion;
        break;
    case Proportion:
        return Contrast;
        break;
    case Contrast:
        return StrokeVariation;
        break;
    case StrokeVariation:
        return ArmStyle;
        break;
    case ArmStyle:
        return Letterform;
        break;
    case Letterform:
        return Midline;
        break;
    case Midline:
        return XHeight;
        break;
    default:
        return InvalidPK;
    }
    return InvalidPK;
}

QString FontStrings::UnicodeCategory(QChar::Category cat)
{
    if (getInstance()->unicodeCategory.contains(cat))
        return getInstance()->unicodeCategory.value(cat);
    return QString();
}

QString FontStrings::scriptTagName(const QString &script)
{
    if (getInstance()->scriptTagNames.contains(script))
        return getInstance()->scriptTagNames.value(script);
    return script;
}

#include "moc_fmfontstrings.cpp"
