/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "fontitem.h"
#include "fmotf.h"
#include "fmglyphsview.h"
#include "typotek.h"
#include "fmbaseshaper.h"

#include <QDebug>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QLocale>
#include <QTextCodec>

#include <QProgressDialog>
#include <QHttp>

#include "QDebug"

#include FT_XFREE86_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TYPE1_TABLES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H

// #include <QWaitCondition>
// #include <QMutex>

int fm_num_face_opened = 0;

FT_Library FontItem::theLibrary = 0;
QGraphicsScene *FontItem::theOneLineScene = 0;

QMap<FT_Encoding, QString> charsetMap;
QMap<int, QString> langIdMap;
int theLocalLangCode;
QStringList name_meaning;
QMap< QString, QMap<int, QString> > panoseMap;
QMap< int, QString > panoseKeys;
QList<int> legitimateNonPathChars;

QVector<QRgb> gray256Palette;
QVector<QRgb> invertedGray256Palette;

// QWaitCondition theCondition;
// QMutex theMutex;

/** functions set for decomposition
 */
static int _moveTo ( const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->moveTo ( to->x, to->y );
	return 0;
}
static int _lineTo ( const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->lineTo ( to->x, to->y );
	return  0;
}
static int _conicTo ( const FT_Vector* control, const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->quadTo ( control->x,control->y,to->x,to->y );
	return 0;
}
static int _cubicTo ( const FT_Vector* control1, const FT_Vector* control2, const FT_Vector*  to, void*   user )
{
	QPainterPath * p = reinterpret_cast<QPainterPath*> ( user );
	p->cubicTo ( control1->x,control1->y,control2->x,control2->y,to->x,to->y );
	return 0;
}

FT_Outline_Funcs outline_funcs=
{
	_moveTo,
	_lineTo,
	_conicTo,
	_cubicTo,
	0,
	0
};
/** **************************************************/


void FontItem::fillCharsetMap()
{
	charsetMap[FT_ENCODING_NONE] = "NONE";
	charsetMap[FT_ENCODING_UNICODE] = "UNICODE";
	charsetMap[FT_ENCODING_MS_SYMBOL] = "MS_SYMBOL";
	charsetMap[FT_ENCODING_SJIS] = "SJIS .";
	charsetMap[FT_ENCODING_GB2312	] = "GB2312 ";
	charsetMap[FT_ENCODING_BIG5] = "BIG5 ";
	charsetMap[FT_ENCODING_WANSUNG] = "WANSUNG ";
	charsetMap[FT_ENCODING_JOHAB] = "JOHAB ";
	charsetMap[FT_ENCODING_ADOBE_LATIN_1] = "ADOBE_LATIN_1 ";
	charsetMap[FT_ENCODING_ADOBE_STANDARD] = "ADOBE_STANDARD ";
	charsetMap[FT_ENCODING_ADOBE_EXPERT] = "ADOBE_EXPERT ";
	charsetMap[FT_ENCODING_ADOBE_CUSTOM] = "ADOBE_CUSTOM ";
	charsetMap[FT_ENCODING_APPLE_ROMAN] = "APPLE_ROMAN ";
	charsetMap[FT_ENCODING_OLD_LATIN_2] = tr ( "This value is deprecated and was never used nor reported by FreeType. Don't use or test for it." );
	charsetMap[FT_ENCODING_MS_SJIS] = "MS_SJIS ";
	charsetMap[FT_ENCODING_MS_GB2312] = "MS_GB2312 ";
	charsetMap[FT_ENCODING_MS_BIG5] = "MS_BIG5 ";
	charsetMap[FT_ENCODING_MS_WANSUNG] = "MS_WANSUNG ";
	charsetMap[FT_ENCODING_MS_JOHAB] = "MS_JOHAB ";
}

void FontItem::fillLegitimateSpaces()
{
	legitimateNonPathChars << 0x0020 ;
	legitimateNonPathChars << 0x00A0 ;
	legitimateNonPathChars << 0x1680 ;
	legitimateNonPathChars << 0x180E ;
	legitimateNonPathChars << 0x2002 ;
	legitimateNonPathChars << 0x2003 ;
	legitimateNonPathChars << 0x2004 ;
	legitimateNonPathChars << 0x2005 ;
	legitimateNonPathChars << 0x2006 ;
	legitimateNonPathChars << 0x2007 ;
	legitimateNonPathChars << 0x2008 ;
	legitimateNonPathChars << 0x2009 ;
	legitimateNonPathChars << 0x200A ;
	legitimateNonPathChars << 0x200B ;
	legitimateNonPathChars << 0x200C ;
	legitimateNonPathChars << 0x200D ;
	legitimateNonPathChars << 0x202F ;
	legitimateNonPathChars << 0x205F ;
	legitimateNonPathChars << 0x2060 ;
	legitimateNonPathChars << 0x3000 ;
	legitimateNonPathChars << 0xFEFF ;
}

void FontItem::fillNamesMeaning()
{
	name_meaning << tr ( "Copyright" )
	<< tr ( "Font Family" )
	<< tr ( "Font Subfamily" )
	<< tr ( "Unique font identifier" )
	<< tr ( "Full font name" )
	<< tr ( "Version string" )
	<< tr ( "Postscript name" )
	<< tr ( "Trademark" )
	<< tr ( "Manufacturer" )
	<< tr ( "Designer" )
	<< tr ( "Description" )
	<< tr ( "URL Vendor" )
	<< tr ( "URL Designer" )
	<< tr ( "License Description" )
	<< tr ( "License Info URL" )
	<< tr ( "Reserved" )
	<< tr ( "Preferred Family" )
	<< tr ( "Preferred Subfamily" )
	<< tr ( "Compatible Full (Macintosh only)" )
	<< tr ( "Sample text" )
	<< tr ( "PostScript CID findfont name" );
}

void FontItem::fillLangIdMap()
{
	if ( langIdMap.count() )
		return;
	langIdMap[0x0000] = "DEFAULT";
	langIdMap[0x0001]="ARABIC_GENERAL";
	langIdMap[0x0401]="ARABIC_SAUDI_ARABIA";
	langIdMap[0x0801]="ARABIC_IRAQ";
	langIdMap[0x0c01]="ARABIC_EGYPT";
	langIdMap[0x1001]="ARABIC_LIBYA";
	langIdMap[0x1401]="ARABIC_ALGERIA";
	langIdMap[0x1801]="ARABIC_MOROCCO";
	langIdMap[0x1c01]="ARABIC_TUNISIA";
	langIdMap[0x2001]="ARABIC_OMAN";
	langIdMap[0x2401]="ARABIC_YEMEN";
	langIdMap[0x2801]="ARABIC_SYRIA";
	langIdMap[0x2c01]="ARABIC_JORDAN";
	langIdMap[0x3001]="ARABIC_LEBANON";
	langIdMap[0x3401]="ARABIC_KUWAIT";
	langIdMap[0x3801]="ARABIC_UAE";
	langIdMap[0x3c01]="ARABIC_BAHRAIN";
	langIdMap[0x4001]="ARABIC_QATAR";
	langIdMap[0x0402]="BULGARIAN_BULGARIA";
	langIdMap[0x0403]="CATALAN_SPAIN";
	langIdMap[0x0004]="CHINESE_GENERAL";
	langIdMap[0x0404]="CHINESE_TAIWAN";
	langIdMap[0x0804]="CHINESE_PRC";
	langIdMap[0x0c04]="CHINESE_HONG_KONG";
	langIdMap[0x1004]="CHINESE_SINGAPORE";
	langIdMap[0x1404]="CHINESE_MACAU";
	langIdMap[TT_MS_LANGID_CHINESE_HONG_KONG]="CHINESE_MACAU";
	langIdMap[0x7C04]="CHINESE_TRADITIONAL";
	langIdMap[0x0405]="CZECH_CZECH_REPUBLIC";
	langIdMap[0x0406]="DANISH_DENMARK";
	langIdMap[0x0407]="GERMAN_GERMANY";
	langIdMap[0x0807]="GERMAN_SWITZERLAND";
	langIdMap[0x0c07]="GERMAN_AUSTRIA";
	langIdMap[0x1007]="GERMAN_LUXEMBOURG";
	langIdMap[0x1407]="GERMAN_LIECHTENSTEI";
	langIdMap[0x0408]="GREEK_GREECE";
	langIdMap[0x2008]="GREEK_GREECE2";
	langIdMap[0x0009]="ENGLISH_GENERAL";
	langIdMap[0x0409]="ENGLISH_UNITED_STATES";
	langIdMap[0x0809]="ENGLISH_UNITED_KINGDOM";
	langIdMap[0x0c09]="ENGLISH_AUSTRALIA";
	langIdMap[0x1009]="ENGLISH_CANADA";
	langIdMap[0x1409]="ENGLISH_NEW_ZEALAND";
	langIdMap[0x1809]="ENGLISH_IRELAND";
	langIdMap[0x1c09]="ENGLISH_SOUTH_AFRICA";
	langIdMap[0x2009]="ENGLISH_JAMAICA";
	langIdMap[0x2409]="ENGLISH_CARIBBEAN";
	langIdMap[0x2809]="ENGLISH_BELIZE";
	langIdMap[0x2c09]="ENGLISH_TRINIDAD";
	langIdMap[0x3009]="ENGLISH_ZIMBABWE";
	langIdMap[0x3409]="ENGLISH_PHILIPPINES";
	langIdMap[0x3809]="ENGLISH_INDONESIA";
	langIdMap[0x3c09]="ENGLISH_HONG_KONG";
	langIdMap[0x4009]="ENGLISH_INDIA";
	langIdMap[0x4409]="ENGLISH_MALAYSIA";
	langIdMap[0x4809]="ENGLISH_SINGAPORE";
	langIdMap[0x040a]="SPANISH_SPAIN_TRADITIONAL_SORT";
	langIdMap[0x080a]="SPANISH_MEXICO";
	langIdMap[0x0c0a]="SPANISH_SPAIN_INTERNATIONAL_SORT";
	langIdMap[0x100a]="SPANISH_GUATEMALA";
	langIdMap[0x140a]="SPANISH_COSTA_RICA";
	langIdMap[0x180a]="SPANISH_PANAMA";
	langIdMap[0x1c0a]="SPANISH_DOMINICAN_REPUBLIC";
	langIdMap[0x200a]="SPANISH_VENEZUELA";
	langIdMap[0x240a]="SPANISH_COLOMBIA";
	langIdMap[0x280a]="SPANISH_PERU";
	langIdMap[0x2c0a]="SPANISH_ARGENTINA";
	langIdMap[0x300a]="SPANISH_ECUADOR";
	langIdMap[0x340a]="SPANISH_CHILE";
	langIdMap[0x380a]="SPANISH_URUGUAY";
	langIdMap[0x3c0a]="SPANISH_PARAGUAY";
	langIdMap[0x400a]="SPANISH_BOLIVIA";
	langIdMap[0x440a]="SPANISH_EL_SALVADOR";
	langIdMap[0x480a]="SPANISH_HONDURAS";
	langIdMap[0x4c0a]="SPANISH_NICARAGUA";
	langIdMap[0x500a]="SPANISH_PUERTO_RICO";
	langIdMap[0x540a]="SPANISH_UNITED_STATES";
	langIdMap[0xE40a]="SPANISH_LATIN_AMERICA";
	langIdMap[0x040b]="FINNISH_FINLAND";
	langIdMap[0x040c]="FRENCH_FRANCE";
	langIdMap[0x080c]="FRENCH_BELGIUM";
	langIdMap[0x0c0c]="FRENCH_CANADA";
	langIdMap[0x100c]="FRENCH_SWITZERLAND";
	langIdMap[0x140c]="FRENCH_LUXEMBOURG";
	langIdMap[0x180c]="FRENCH_MONACO";
	langIdMap[0x1c0c]="FRENCH_WEST_INDIES";
	langIdMap[0x200c]="FRENCH_REUNION";
	langIdMap[0x240c]="FRENCH_CONGO";
	langIdMap[TT_MS_LANGID_FRENCH_CONGO]="FRENCH_ZAIRE";
	langIdMap[0x280c]="FRENCH_SENEGAL";
	langIdMap[0x2c0c]="FRENCH_CAMEROON";
	langIdMap[0x300c]="FRENCH_COTE_D_IVOIRE";
	langIdMap[0x340c]="FRENCH_MALI";
	langIdMap[0x380c]="FRENCH_MOROCCO";
	langIdMap[0x3c0c]="FRENCH_HAITI";
	langIdMap[0xE40c]="FRENCH_NORTH_AFRICA";
	langIdMap[0x040d]="HEBREW_ISRAEL";
	langIdMap[0x040e]="HUNGARIAN_HUNGARY";
	langIdMap[0x040f]="ICELANDIC_ICELAND";
	langIdMap[0x0410]="ITALIAN_ITALY";
	langIdMap[0x0810]="ITALIAN_SWITZERLAND";
	langIdMap[0x0411]="JAPANESE_JAPAN";
	langIdMap[0x0412]="KOREAN_EXTENDED_WANSUNG_KOREA";
	langIdMap[0x0812]="KOREAN_JOHAB_KOREA";
	langIdMap[0x0413]="DUTCH_NETHERLANDS";
	langIdMap[0x0813]="DUTCH_BELGIUM";
	langIdMap[0x0414]="NORWEGIAN_NORWAY_BOKMAL";
	langIdMap[0x0814]="NORWEGIAN_NORWAY_NYNORSK";
	langIdMap[0x0415]="POLISH_POLAND";
	langIdMap[0x0416]="PORTUGUESE_BRAZIL";
	langIdMap[0x0816]="PORTUGUESE_PORTUGAL";
	langIdMap[0x0417]="RHAETO_ROMANIC_SWITZERLAND";
	langIdMap[0x0418]="ROMANIAN_ROMANIA";
	langIdMap[0x0818]="MOLDAVIAN_MOLDAVIA";
	langIdMap[0x0419]="RUSSIAN_RUSSIA";
	langIdMap[0x0819]="RUSSIAN_MOLDAVIA";
	langIdMap[0x041a]="CROATIAN_CROATIA";
	langIdMap[0x081a]="SERBIAN_SERBIA_LATIN";
	langIdMap[0x0c1a]="SERBIAN_SERBIA_CYRILLIC";
	langIdMap[0x101a]="BOSNIAN_BOSNIA_HERZEGOVINA";
	langIdMap[0x101a]="CROATIAN_BOSNIA_HERZEGOVINA";
	langIdMap[0x141a]="BOSNIAN_BOSNIA_HERZEGOVINA";
	langIdMap[0x181a]="SERBIAN_BOSNIA_HERZ_LATIN";
	langIdMap[0x181a]="SERBIAN_BOSNIA_HERZ_CYRILLIC";
	langIdMap[0x041b]="SLOVAK_SLOVAKIA";
	langIdMap[0x041c]="ALBANIAN_ALBANIA";
	langIdMap[0x041d]="SWEDISH_SWEDEN";
	langIdMap[0x081d]="SWEDISH_FINLAND";
	langIdMap[0x041e]="THAI_THAILAND";
	langIdMap[0x041f]="TURKISH_TURKEY";
	langIdMap[0x0420]="URDU_PAKISTAN";
	langIdMap[0x0820]="URDU_INDIA";
	langIdMap[0x0421]="INDONESIAN_INDONESIA";
	langIdMap[0x0422]="UKRAINIAN_UKRAINE";
	langIdMap[0x0423]="BELARUSIAN_BELARUS";
	langIdMap[0x0424]="SLOVENE_SLOVENIA";
	langIdMap[0x0425]="ESTONIAN_ESTONIA";
	langIdMap[0x0426]="LATVIAN_LATVIA";
	langIdMap[0x0427]="LITHUANIAN_LITHUANIA";
	langIdMap[0x0827]="CLASSIC_LITHUANIAN_LITHUANIA";
	langIdMap[0x0428]="TAJIK_TAJIKISTAN";
	langIdMap[0x0429]="FARSI_IRAN";
	langIdMap[0x042a]="VIETNAMESE_VIET_NAM";
	langIdMap[0x042b]="ARMENIAN_ARMENIA";
	langIdMap[0x042c]="AZERI_AZERBAIJAN_LATIN";
	langIdMap[0x082c]="AZERI_AZERBAIJAN_CYRILLIC";
	langIdMap[0x042d]="BASQUE_SPAIN";
	langIdMap[0x042e]="SORBIAN_GERMANY";
	langIdMap[0x042f]="MACEDONIAN_MACEDONIA";
	langIdMap[0x0430]="SUTU_SOUTH_AFRICA";
	langIdMap[0x0431]="TSONGA_SOUTH_AFRICA";
	langIdMap[0x0432]="TSWANA_SOUTH_AFRICA";
	langIdMap[0x0433]="VENDA_SOUTH_AFRICA";
	langIdMap[0x0434]="XHOSA_SOUTH_AFRICA";
	langIdMap[0x0435]="ZULU_SOUTH_AFRICA";
	langIdMap[0x0436]="AFRIKAANS_SOUTH_AFRICA";
	langIdMap[0x0437]="GEORGIAN_GEORGIA";
	langIdMap[0x0438]="FAEROESE_FAEROE_ISLANDS";
	langIdMap[0x0439]="HINDI_INDIA";
	langIdMap[0x043a]="MALTESE_MALTA";
	langIdMap[0x043b]="SAMI_NORTHERN_NORWAY";
	langIdMap[0x083b]="SAMI_NORTHERN_SWEDEN";
	langIdMap[0x0C3b]="SAMI_NORTHERN_FINLAND";
	langIdMap[0x103b]="SAMI_LULE_NORWAY";
	langIdMap[0x143b]="SAMI_LULE_SWEDEN";
	langIdMap[0x183b]="SAMI_SOUTHERN_NORWAY";
	langIdMap[0x1C3b]="SAMI_SOUTHERN_SWEDEN";
	langIdMap[0x203b]="SAMI_SKOLT_FINLAND";
	langIdMap[0x243b]="SAMI_INARI_FINLAND";
	langIdMap[0x043b]="SAAMI_LAPONIA";
	langIdMap[0x043c]="IRISH_GAELIC_IRELAND";
	langIdMap[0x083c]="SCOTTISH_GAELIC_UNITED_KINGDOM";
	langIdMap[0x083c]="SCOTTISH_GAELIC_UNITED_KINGDOM";
	langIdMap[0x043c]="IRISH_GAELIC_IRELAND";
	langIdMap[0x043d]="YIDDISH_GERMANY";
	langIdMap[0x043e]="MALAY_MALAYSIA";
	langIdMap[0x083e]="MALAY_BRUNEI_DARUSSALAM";
	langIdMap[0x043f]="KAZAK_KAZAKSTAN";
	langIdMap[0x0440]="KIRGHIZ_KIRGHIZSTAN";
	langIdMap[0x0441]="SWAHILI_KENYA";
	langIdMap[0x0442]="TURKMEN_TURKMENISTAN";
	langIdMap[0x0443]="UZBEK_UZBEKISTAN_LATIN";
	langIdMap[0x0843]="UZBEK_UZBEKISTAN_CYRILLIC";
	langIdMap[0x0444]="TATAR_TATARSTAN";
	langIdMap[0x0445]="BENGALI_INDIA";
	langIdMap[0x0845]="BENGALI_BANGLADESH";
	langIdMap[0x0446]="PUNJABI_INDIA";
	langIdMap[0x0846]="PUNJABI_ARABIC_PAKISTAN";
	langIdMap[0x0447]="GUJARATI_INDIA";
	langIdMap[0x0448]="ORIYA_INDIA";
	langIdMap[0x0449]="TAMIL_INDIA";
	langIdMap[0x044a]="TELUGU_INDIA";
	langIdMap[0x044b]="KANNADA_INDIA";
	langIdMap[0x044c]="MALAYALAM_INDIA";
	langIdMap[0x044d]="ASSAMESE_INDIA";
	langIdMap[0x044e]="MARATHI_INDIA";
	langIdMap[0x044f]="SANSKRIT_INDIA";
	langIdMap[0x0450]="MONGOLIAN_MONGOLIA/*Cyrillic*/";
	langIdMap[0x0850]="MONGOLIAN_MONGOLIA_MONGOLIAN";
	langIdMap[0x0451]="TIBETAN_CHINA";
	/*TT_MS_LANGID_TIBETAN_BHUTANiscorrect,BTW.*/
	langIdMap[0x0851]="DZONGHKA_BHUTAN";
	langIdMap[0x0451]="TIBETAN_BHUTAN";
	langIdMap[TT_MS_LANGID_DZONGHKA_BHUTAN]="TIBETAN_BHUTAN";
	langIdMap[0x0452]="WELSH_WALES";
	langIdMap[0x0453]="KHMER_CAMBODIA";
	langIdMap[0x0454]="LAO_LAOS";
	langIdMap[0x0455]="BURMESE_MYANMAR";
	langIdMap[0x0456]="GALICIAN_SPAIN";
	langIdMap[0x0457]="KONKANI_INDIA";
	langIdMap[0x0458]="MANIPURI_INDIA/*Bengali*/";
	langIdMap[0x0459]="SINDHI_INDIA/*Arabic*/";
	langIdMap[0x0859]="SINDHI_PAKISTAN";
	langIdMap[0x045a]="SYRIAC_SYRIA";
	langIdMap[0x045b]="SINHALESE_SRI_LANKA";
	langIdMap[0x045c]="CHEROKEE_UNITED_STATES";
	langIdMap[0x045d]="INUKTITUT_CANADA";
	langIdMap[0x045e]="AMHARIC_ETHIOPIA";
	langIdMap[0x045f]="TAMAZIGHT_MOROCCO/*Arabic*/";
	langIdMap[0x085f]="TAMAZIGHT_MOROCCO_LATIN";
	langIdMap[0x0460]="KASHMIRI_PAKISTAN/*Arabic*/";
	langIdMap[0x0860]="KASHMIRI_SASIA";
	langIdMap[TT_MS_LANGID_KASHMIRI_SASIA]="KASHMIRI_INDIA";
	langIdMap[0x0461]="NEPALI_NEPAL";
	langIdMap[0x0861]="NEPALI_INDIA";
	langIdMap[0x0462]="FRISIAN_NETHERLANDS";
	langIdMap[0x0463]="PASHTO_AFGHANISTAN";
	langIdMap[0x0464]="FILIPINO_PHILIPPINES";
	langIdMap[0x0465]="DHIVEHI_MALDIVES";
	langIdMap[TT_MS_LANGID_DHIVEHI_MALDIVES]="DIVEHI_MALDIVES";
	langIdMap[0x0466]="EDO_NIGERIA";
	langIdMap[0x0467]="FULFULDE_NIGERIA";
	langIdMap[0x0468]="HAUSA_NIGERIA";
	langIdMap[0x0469]="IBIBIO_NIGERIA";
	langIdMap[0x046a]="YORUBA_NIGERIA";
	langIdMap[0x046b]="QUECHUA_BOLIVIA";
	langIdMap[0x086b]="QUECHUA_ECUADOR";
	langIdMap[0x0c6b]="QUECHUA_PERU";
	langIdMap[0x046c]="SEPEDI_SOUTH_AFRICA";
	langIdMap[0x0470]="IGBO_NIGERIA";
	langIdMap[0x0471]="KANURI_NIGERIA";
	langIdMap[0x0472]="OROMO_ETHIOPIA";
	langIdMap[0x0473]="TIGRIGNA_ETHIOPIA";
	langIdMap[0x0873]="TIGRIGNA_ERYTHREA";
	langIdMap[TT_MS_LANGID_TIGRIGNA_ERYTHREA]="TIGRIGNA_ERYTREA";
	langIdMap[0x0474]="GUARANI_PARAGUAY";
	langIdMap[0x0475]="HAWAIIAN_UNITED_STATES";
	langIdMap[0x0476]="LATIN";
	langIdMap[0x0477]="SOMALI_SOMALIA";
	langIdMap[0x0478]="YI_CHINA";
	langIdMap[0x0479]="PAPIAMENTU_NETHERLANDS_ANTILLES";
	langIdMap[0x0480]="UIGHUR_CHINA";
	langIdMap[0x0481]="MAORI_NEW_ZEALAND";
	langIdMap[0x04ff]="HUMAN_INTERFACE_DEVICE";

	// Now, what’s the locale code? so we’ll be able to get rid of unused data
	QString sysLang ( QLocale::languageToString ( QLocale::system ().language() ).toUpper() );
	QString sysCountry ( QLocale::countryToString ( QLocale::system ().country() ).toUpper() );
	int langIdMatch ( 0 );
	QMap<int,QString>::const_iterator lit;
	QList<int> locCodes;
	for ( lit = langIdMap.constBegin(); lit != langIdMap.constEnd() ;++lit )
	{
		if ( lit.value().startsWith ( sysLang ) )
			locCodes << lit.key();
	}
	langIdMatch = locCodes.value ( 0 );
	for ( int i ( 0 ); i < locCodes.count(); ++i )
	{
		if ( langIdMap[locCodes.at ( i ) ].contains ( sysCountry ) )
		{
			langIdMatch = locCodes.at ( i );
		}
	}

	theLocalLangCode = langIdMatch;
	qDebug() <<"LANG"<<theLocalLangCode<<langIdMap[theLocalLangCode];
}

void FontItem::fill256Palette()
{
// #ifdef   PLATFORM_APPLE
// 	for ( int i = 0; i < 256 ; ++i )
// 	{
// 		gray256Palette << qRgb (255-i, 255-i,255- i );
// 	}
// #else
	for ( int i = 0; i < 256 ; ++i )
	{
		gray256Palette << qRgba ( 0,0,0, i );
	}
// #endif
}

void FontItem::fillInvertedPalette()
{
	for ( int i = 0; i < 256 ; ++i )
	{
		invertedGray256Palette << qRgb ( i , i,  i );
	}
}

void FontItem::fillPanoseMap()
{
	// http://www.microsoft.com/OpenType/OTSpec/os2ver0.htm#pan

	QMap<int, QString> mapModel;
	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Text and Display" ) ;
	mapModel[ 3 ] = tr ( "Script" ) ;
	mapModel[ 4 ] = tr ( "Decorative" ) ;
	mapModel[ 5 ] = tr ( "Pictorial" ) ;

	panoseMap[tr ( "Family Type" ) ] = mapModel;
	panoseKeys[0] = tr ( "Family Type" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Cove" ) ;
	mapModel[ 3 ] = tr ( "Obtuse Cove" ) ;
	mapModel[ 4 ] = tr ( "Square Cove" ) ;
	mapModel[ 5 ] = tr ( "Obtuse Square Cove" ) ;
	mapModel[ 6 ] = tr ( "Square" ) ;
	mapModel[ 7 ] = tr ( "Thin" ) ;
	mapModel[ 8 ] = tr ( "Bone" ) ;
	mapModel[ 9 ] = tr ( "Exaggerated" ) ;
	mapModel[ 10 ] = tr ( "Triangle" ) ;
	mapModel[ 11 ] = tr ( "Normal Sans" ) ;
	mapModel[ 12 ] = tr ( "Obtuse Sans" ) ;
	mapModel[ 13 ] = tr ( "Perp Sans" ) ;
	mapModel[ 14 ] = tr ( "Flared" ) ;
	mapModel[ 15 ] = tr ( "Rounded" ) ;

	panoseMap[tr ( "Serif style" ) ] = mapModel;
	panoseKeys[1] = tr ( "Serif style" ) ;
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Very Light" ) ;
	mapModel[ 3 ] = tr ( "Light" ) ;
	mapModel[ 4 ] = tr ( "Thin" ) ;
	mapModel[ 5 ] = tr ( "Book" ) ;
	mapModel[ 6 ] = tr ( "Medium" ) ;
	mapModel[ 7 ] = tr ( "Demi" ) ;
	mapModel[ 8 ] = tr ( "Bold" ) ;
	mapModel[ 9 ] = tr ( "Heavy" ) ;
	mapModel[ 10 ] = tr ( "Black" ) ;
	mapModel[ 11 ] = tr ( "Nord" ) ;

	panoseMap[tr ( "Weight" ) ] = mapModel;
	panoseKeys[2] = tr ( "Weight" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Old Style" ) ;
	mapModel[ 3 ] = tr ( "Modern" ) ;
	mapModel[ 4 ] = tr ( "Even Width" ) ;
	mapModel[ 5 ] = tr ( "Expanded" ) ;
	mapModel[ 6 ] = tr ( "Condensed" ) ;
	mapModel[ 7 ] = tr ( "Very Expanded" ) ;
	mapModel[ 8 ] = tr ( "Very Condensed" ) ;
	mapModel[ 9 ] = tr ( "Monospaced" ) ;

	panoseMap[tr ( "Proportion" ) ] = mapModel;
	panoseKeys[3] = tr ( "Proportion" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "None" ) ;
	mapModel[ 3 ] = tr ( "Very Low" ) ;
	mapModel[ 4 ] = tr ( "Low" ) ;
	mapModel[ 5 ] = tr ( "Medium Low" ) ;
	mapModel[ 6 ] = tr ( "Medium" ) ;
	mapModel[ 7 ] = tr ( "Medium High" ) ;
	mapModel[ 8 ] = tr ( "High" ) ;
	mapModel[ 9 ] = tr ( "Very High" ) ;

	panoseMap[tr ( "Contrast" ) ] = mapModel;
	panoseKeys[4] = tr ( "Contrast" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Gradual/Diagonal" ) ;
	mapModel[ 3 ] = tr ( "Gradual/Transitional" ) ;
	mapModel[ 4 ] = tr ( "Gradual/Vertical" ) ;
	mapModel[ 5 ] = tr ( "Gradual/Horizontal" ) ;
	mapModel[ 6 ] = tr ( "Rapid/Vertical" ) ;
	mapModel[ 7 ] = tr ( "Rapid/Horizontal" ) ;
	mapModel[ 8 ] = tr ( "Instant/Vertical" ) ;

	panoseMap[tr ( "Stroke Variation" ) ] = mapModel;
	panoseKeys[5] = tr ( "Stroke Variation" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Straight Arms/Horizontal" ) ;
	mapModel[ 3 ] = tr ( "Straight Arms/Wedge" ) ;
	mapModel[ 4 ] = tr ( "Straight Arms/Vertical" ) ;
	mapModel[ 5 ] = tr ( "Straight Arms/Single Serif" ) ;
	mapModel[ 6 ] = tr ( "Straight Arms/Double Serif" ) ;
	mapModel[ 7 ] = tr ( "Non-Straight Arms/Horizontal" ) ;
	mapModel[ 8 ] = tr ( "Non-Straight Arms/Wedge" ) ;
	mapModel[ 9 ] = tr ( "Non-Straight Arms/Vertical" ) ;
	mapModel[ 10 ] = tr ( "Non-Straight Arms/Single Serif" ) ;
	mapModel[ 11 ] = tr ( "Non-Straight Arms/Double Serif" ) ;

	panoseMap[tr ( "Arm Style" ) ] = mapModel;
	panoseKeys[6] = tr ( "Arm Style" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Normal/Contact" ) ;
	mapModel[ 3 ] = tr ( "Normal/Weighted" ) ;
	mapModel[ 4 ] = tr ( "Normal/Boxed" ) ;
	mapModel[ 5 ] = tr ( "Normal/Flattened" ) ;
	mapModel[ 6 ] = tr ( "Normal/Rounded" ) ;
	mapModel[ 7 ] = tr ( "Normal/Off Center" ) ;
	mapModel[ 8 ] = tr ( "Normal/Square" ) ;
	mapModel[ 9 ] = tr ( "Oblique/Contact" ) ;
	mapModel[ 10 ] = tr ( "Oblique/Weighted" ) ;
	mapModel[ 11 ] = tr ( "Oblique/Boxed" ) ;
	mapModel[ 12 ] = tr ( "Oblique/Flattened" ) ;
	mapModel[ 13 ] = tr ( "Oblique/Rounded" ) ;
	mapModel[ 14 ] = tr ( "Oblique/Off Center" ) ;
	mapModel[ 15 ] = tr ( "Oblique/Square" ) ;

	panoseMap[tr ( "Letterform" ) ] = mapModel;
	panoseKeys[7] = tr ( "Letterform" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Standard/Trimmed" ) ;
	mapModel[ 3 ] = tr ( "Standard/Pointed" ) ;
	mapModel[ 4 ] = tr ( "Standard/Serifed" ) ;
	mapModel[ 5 ] = tr ( "High/Trimmed" ) ;
	mapModel[ 6 ] = tr ( "High/Pointed" ) ;
	mapModel[ 7 ] = tr ( "High/Serifed" ) ;
	mapModel[ 8 ] = tr ( "Constant/Trimmed" ) ;
	mapModel[ 9 ] = tr ( "Constant/Pointed" ) ;
	mapModel[ 10 ] = tr ( "Constant/Serifed" ) ;
	mapModel[ 11 ] = tr ( "Low/Trimmed" ) ;
	mapModel[ 12 ] = tr ( "Low/Pointed" ) ;
	mapModel[ 13 ] = tr ( "Low/Serifed" ) ;

	panoseMap[tr ( "Midline" ) ] = mapModel;
	panoseKeys[8] = tr ( "Midline" );
	mapModel.clear();

	mapModel[ 0 ] = tr ( "Any" ) ;
	mapModel[ 1 ] = tr ( "No Fit" ) ;
	mapModel[ 2 ] = tr ( "Constant/Small" ) ;
	mapModel[ 3 ] = tr ( "Constant/Standard" ) ;
	mapModel[ 4 ] = tr ( "Constant/Large" ) ;
	mapModel[ 5 ] = tr ( "Ducking/Small" ) ;
	mapModel[ 6 ] = tr ( "Ducking/Standard" ) ;
	mapModel[ 7 ] = tr ( "Ducking/Large" ) ;

	panoseMap[tr ( "X-Height" ) ] = mapModel;
	panoseKeys[9] = tr ( "X-Height" );
}

FontItem::FontItem ( QString path , bool remote, bool faststart )
{
	m_valid = false;
	m_remote = remote;
	remoteCached = false;
	stopperDownload = false;
	m_face = 0;
	facesRef = 0;
	m_glyphsPerRow = 5;
	hasUnicode = false;
	currentChar = -1;
	m_isOpenType = false;
	otf = 0;
	m_rasterFreetype = false;
	m_progression = PROGRESSION_LTR;
	m_shaperType = 1;
	renderReturnWidth = false;

	if ( langIdMap.isEmpty() )
		fillLangIdMap();
	if ( charsetMap.isEmpty() )
		fillCharsetMap();
	if ( legitimateNonPathChars.isEmpty() )
		fillLegitimateSpaces();
	if ( gray256Palette.isEmpty() )
		fill256Palette();
	if ( invertedGray256Palette.isEmpty() )
		fillInvertedPalette();
	if ( panoseMap.isEmpty() )
		fillPanoseMap();

	if ( !theOneLineScene )
	{
		theOneLineScene = new QGraphicsScene;
	}

	allIsRendered = false;
	m_path = path;

	if ( m_remote || faststart )
	{
		m_valid = true;
		return;
	}

	QFileInfo infopath ( m_path );
	m_name = infopath.fileName();



	if ( ! ensureFace() )
	{
		return;
	}


	if ( infopath.suffix() == "pfb" || infopath.suffix() == "PFB" )
	{
		m_afm = m_path;
		if ( infopath.suffix() == "pfb" )
		{
			m_afm.replace ( ".pfb",".afm" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".afm" ,".AFM" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
		else if ( infopath.suffix() == "PFB" )
		{
			m_afm.replace ( ".PFB",".AFM" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".AFM" ,".afm" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
	}


	if ( testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" )
	{
		m_isOpenType = true;
	}

	m_type = FT_Get_X11_Font_Format ( m_face ); 
	if ( typotek::getInstance()->familySchemeFreetype() || !m_isOpenType )
	{
		m_family = m_face->family_name;
		m_variant = m_face->style_name;
	}
	else
	{
		m_family = getAlternateFamilyName();
		m_variant = getAlternateVariantName();
	}
	m_numGlyphs = m_face->num_glyphs;
	m_numFaces = m_face->num_faces;

	for ( int i = 0 ;i < m_face->num_charmaps; ++i )
	{
		m_charsets << charsetMap[m_face->charmaps[i]->encoding];
	}

// 	m_charsets = m_charsets.toSet().toList();

	m_lock = false;
	pixList.clear();
	sceneList.clear();

	if ( m_family.isEmpty() )
		return;
	if ( m_variant.isEmpty() )
		return;

	m_valid = true;
	releaseFace();

}

void FontItem::updateItem()
{
	QFileInfo infopath ( m_path );
	m_name = infopath.fileName();



	if ( ! ensureFace() )
	{
		return;
	}


	if ( infopath.suffix() == "pfb" || infopath.suffix() == "PFB" )
	{
		m_afm = m_path;
		if ( infopath.suffix() == "pfb" )
		{
			m_afm.replace ( ".pfb",".afm" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".afm" ,".AFM" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
		else if ( infopath.suffix() == "PFB" )
		{
			m_afm.replace ( ".PFB",".AFM" );
			if ( !QFile::exists ( m_afm ) )
			{
				m_afm.replace ( ".AFM" ,".afm" );
				if ( !QFile::exists ( m_afm ) )
				{
					m_afm = "";
				}
			}
		}
	}


	if ( testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" )
	{
		m_isOpenType = true;
	}

	m_type = FT_Get_X11_Font_Format ( m_face );
	m_family = m_face->family_name;
	m_variant = m_face->style_name;
	m_numGlyphs = m_face->num_glyphs;
	m_numFaces = m_face->num_faces;

	for ( int i = 0 ;i < m_face->num_charmaps; ++i )
	{
		m_charsets << charsetMap[m_face->charmaps[i]->encoding];
	}

	m_charsets = m_charsets.toSet().toList();

	releaseFace();
}

FontItem::~FontItem()
{
	if ( m_isOpenType && otf )
	{
// 		delete otf;
	}
}

bool FontItem::ensureLibrary()
{
	if ( theLibrary )
		return true;
	ft_error = FT_Init_FreeType ( &theLibrary );
	if ( ft_error )
	{
		qDebug() << "Error loading ft_library ";
		return false;
	}
	return true;
}

bool FontItem::ensureFace()
{
// 	qDebug("ENSUREFACE") ;
	if ( ensureLibrary() )
	{
		if ( m_face )
		{
			++facesRef;
			return true;
		}
		QString trueFile ( m_remote ? remoteHerePath : m_path );
		ft_error = FT_New_Face ( theLibrary, trueFile.toLocal8Bit() , 0, &m_face );
		if ( ft_error )
		{
			qDebug() << "Error loading face [" << trueFile <<"]";
			return false;
		}
		ft_error = FT_Select_Charmap ( m_face, FT_ENCODING_UNICODE );
		if ( ft_error )
		{
			hasUnicode = false;
		}
		else
		{
			hasUnicode = true;
		}
		if ( spaceIndex.isEmpty() )
		{
			int gIndex ( 0 );
			for ( int i ( 0 ); i < legitimateNonPathChars.count(); ++i )
			{
				gIndex =   FT_Get_Char_Index ( m_face , legitimateNonPathChars[i] );
				if ( gIndex )
				{
					spaceIndex << gIndex;
				}
			}
		}
		m_glyph = m_face->glyph;
		++facesRef;
		++fm_num_face_opened;
		return true;
	}
	return false;
}

void FontItem::releaseFace()
{
	if ( m_face )
	{
		--facesRef;
		if ( facesRef == 0 )
		{
			FT_Done_Face ( m_face );
			m_face = 0;
			--fm_num_face_opened;
		}
	}
}


QString FontItem::testFlag ( long flag, long against, QString yes, QString no )
{
	if ( ( flag & against ) == against )
		return yes;
	else
		return no;
}

QString FontItem::value ( QString k )
{
	// I don’t know if something relies o it so I keep it, for the moment.
	if ( k == "family" )
		return m_family;
	else if ( k == "variant" )
		return m_variant;

	// 0 is default language
	// TODO inspect all available languages
	QMap<QString, QString> namap ( moreInfo.value ( 0 ) );
	return namap.value ( k );
}

QString FontItem::panose ( QString k )
{
	return panoseInfo.value ( k );
}


QString FontItem::name()
{
	return m_name;
}

QGraphicsPathItem * FontItem::itemFromChar ( int charcode, double size )
{

	uint glyphIndex = 0;
	currentChar = charcode;
	glyphIndex = FT_Get_Char_Index ( m_face, charcode );

	return itemFromGindex ( glyphIndex,size );

}

QGraphicsPathItem * FontItem::itemFromGindex ( int index, double size )
{
	int charcode = index ;
	double scalefactor = size / m_face->units_per_EM;
	ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_NO_SCALE );
	if ( ft_error )
	{
		QPainterPath glyphPath;
		glyphPath.addRect ( 0.0,0.0, size, size );
		QGraphicsPathItem *glyph = new  QGraphicsPathItem;
		glyph->setBrush ( QBrush ( Qt::red ) );
		glyph->setPath ( glyphPath );
		glyph->setData ( GLYPH_DATA_HADVANCE , ( double ) size );
		return glyph;
	}

	FT_Outline *outline = &m_glyph->outline;
	QPainterPath glyphPath ( QPointF ( 0.0,0.0 ) );
	FT_Outline_Decompose ( outline, &outline_funcs, &glyphPath );
	QGraphicsPathItem *glyph = new  QGraphicsPathItem;

	if ( glyphPath.elementCount() < 3 && !spaceIndex.contains ( index ) )
	{
		QBrush brush ( Qt::SolidPattern );
		brush.setColor ( Qt::red );
		QPen pen ( brush, 0 );
		QPainterPath errPath;
		errPath.addRect ( 0.0,-size, size , size );
		glyph->setBrush ( brush );
		glyph->setPen ( pen );
		glyph->setPath ( errPath );
		glyph->setData ( GLYPH_DATA_HADVANCE , ( double ) size  /scalefactor );
	}
	else
	{
		glyph->setBrush ( QBrush ( Qt::SolidPattern ) );
		glyph->setPath ( glyphPath );
		glyph->setData ( GLYPH_DATA_HADVANCE , ( double ) m_glyph->metrics.horiAdvance );
		glyph->setData ( 5, ( double ) m_glyph->metrics.vertAdvance );
		glyph->scale ( scalefactor,-scalefactor );
	}
	return glyph;
}

QGraphicsPixmapItem * FontItem::itemFromCharPix ( int charcode, double size )
{
	uint glyphIndex = 0;
	currentChar = charcode;
	glyphIndex = FT_Get_Char_Index ( m_face, charcode );

	return itemFromGindexPix ( glyphIndex,size );

}


QGraphicsPixmapItem * FontItem::itemFromGindexPix ( int index, double size )
{
	int charcode = index ;

	// Grab metrics in FONT UNIT
	ft_error = FT_Load_Glyph ( m_face,
	                           charcode  ,
	                           FT_LOAD_NO_SCALE );
	if ( ft_error )
	{
		QPixmap square ( size , size );
		square.fill ( Qt::red );
		QGraphicsPixmapItem *glyph = new QGraphicsPixmapItem ( square );
		glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setData ( GLYPH_DATA_HADVANCE ,size / ( size / m_face->units_per_EM ) );
		return glyph;
	}

	double takeAdvanceBeforeRender = m_glyph->metrics.horiAdvance * ( ( double ) QApplication::desktop()->physicalDpiX() / 72.0 );
	double takeVertAdvanceBeforeRender = m_glyph->metrics.vertAdvance * ( ( double ) QApplication::desktop()->physicalDpiX() / 72.0 );
	double takeLeftBeforeRender = ( double ) m_glyph->metrics.horiBearingX * ( ( double ) QApplication::desktop()->physicalDpiX() / 72.0 );

	// Set size
	FT_Set_Char_Size ( m_face,
	                   size  * 64 ,
	                   0,
	                   QApplication::desktop()->physicalDpiX(),
	                   QApplication::desktop()->physicalDpiY() );

	// Reload the glyph in device dependant way
	ft_error = FT_Load_Glyph ( m_face,
	                           charcode  ,
	                           FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING );
	if ( ft_error )
	{
		QPixmap square ( size , size );
		square.fill ( Qt::red );
		QGraphicsPixmapItem *glyph = new QGraphicsPixmapItem ( square );
		glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setData ( GLYPH_DATA_HADVANCE ,size / ( size / m_face->units_per_EM ) );
		return glyph;
	}

	// Render the glyph into a grayscale bitmap
	ft_error = FT_Render_Glyph ( m_face->glyph,
	                             FT_RENDER_MODE_NORMAL );
	if ( ft_error )
	{
		QPixmap square ( size , size );
		square.fill ( Qt::red );
		QGraphicsPixmapItem *glyph = new QGraphicsPixmapItem ( square );
		glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setData ( GLYPH_DATA_HADVANCE ,size  / ( size / m_face->units_per_EM ) );
		return glyph;
	}


	QImage img ( glyphImage() );
	QGraphicsPixmapItem *glyph = new  QGraphicsPixmapItem;

	if ( img.isNull() && !spaceIndex.contains ( index ) )
	{
		QPixmap square ( size , size );
		square.fill ( Qt::red );
		glyph->setPixmap ( square );
		glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , 0 );
		glyph->setData ( GLYPH_DATA_BITMAPTOP,size );
		glyph->setData ( GLYPH_DATA_HADVANCE ,size / ( size / m_face->units_per_EM ) );
	}
	else
	{
#ifndef PLATFORM_APPLE
		glyph->setPixmap ( QPixmap::fromImage ( img ) );
#else
		QPixmap aPix ( img.width(), img.height() );
		aPix.fill ( QColor ( 0,0,0,0 ) );
		QPainter aPainter ( &aPix );
		aPainter.drawImage ( 0,0, img );
		glyph->setPixmap ( aPix );
#endif
		// we need to transport more data
		glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
		glyph->setData ( GLYPH_DATA_BITMAPLEFT , takeLeftBeforeRender );
		glyph->setData ( GLYPH_DATA_BITMAPTOP , m_face->glyph->bitmap_top );
		glyph->setData ( GLYPH_DATA_HADVANCE , takeAdvanceBeforeRender );
		glyph->setData ( GLYPH_DATA_VADVANCE , takeVertAdvanceBeforeRender );
	}

	return glyph;
}


/// Nature line
double FontItem::renderLine ( QGraphicsScene * scene,
                              QString spec,
                              QPointF origine,
                              double lineWidth,
                              double fsize ,
                              double zindex ,
                              bool record )
{
// 	qDebug() <<fancyName() <<"::"<<"renderLine("<<scene<<spec<<lineWidth<<fsize<<zindex<<record<<")";
	double retValue ( 0.0 );
	if ( spec.isEmpty() )
		return retValue;

	ensureFace();

	if ( record )
		sceneList.append ( scene );
	double sizz = fsize;
	double scalefactor = sizz / m_face->units_per_EM;
	double pWidth = lineWidth ;
	const double distance = 20;
	QPointF pen ( origine );
	if ( m_rasterFreetype )
	{
		QList<QGraphicsPixmapItem*> mayBeRemoved;
		for ( int i=0; i < spec.length(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromCharPix ( spec.at ( i ).unicode(), sizz );
			if( spec.at(i).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}
			if ( !glyph )
			{
				continue;
			}
			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= ( glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() ) * scalefactor;
				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;

			/************************************/
			if ( record )
				pixList.append ( glyph );
			
			mayBeRemoved.append(glyph);
			
			scene->addItem ( glyph );
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue += 1;
			
			glyph->setPos ( pen.x() + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() * scalefactor, pen.y() - glyph->data ( GLYPH_DATA_BITMAPTOP ).toInt() );
			glyph->setZValue ( zindex );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			/************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else if ( m_progression == PROGRESSION_TTB )
			{
				pen.ry() +=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
			}
		}
	}
	else
	{
		QList<QGraphicsPathItem*> mayBeRemoved;
		for ( int i=0; i < spec.length(); ++i )
		{
			if ( !scene->sceneRect().contains ( pen ) && record )
				break;
			QGraphicsPathItem *glyph = itemFromChar ( spec.at ( i ).unicode(), sizz );
			if ( !glyph )
				continue;
			if( spec.at(i).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}
			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;

				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}

			/*********************************/
			if ( record )
				glyphList.append ( glyph );
			scene->addItem ( glyph );
			
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue += 1;
			
			glyph->setPos ( pen );
			glyph->setZValue ( zindex );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			/*********************************/

			if ( m_progression == PROGRESSION_LTR )
			{
				pen.rx() += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pen.ry() +=  glyph->data ( GLYPH_DATA_VADVANCE ).toDouble() * scalefactor;
			}
		}
	}

	releaseFace();
	return retValue;
}

/// Featured line
double FontItem::renderLine ( OTFSet set, QGraphicsScene * scene, QString spec, QPointF origine,double lineWidth, double fsize, bool record )
{
// 	qDebug()<<"Featured("<< spec <<")";
	double retValue ( 0.0 );
	if ( spec.isEmpty() )
		return retValue;
	if ( !m_isOpenType )
		return retValue;
	ensureFace();

	otf = new FMOtf ( m_face, 0x10000 );// You think "What’s this 0x10000?", so am I! Just accept Harfbuzz black magic :)
	if ( !otf )
		return retValue;
	if ( record )
		sceneList.append ( scene );
	double sizz = fsize;
	double scalefactor = sizz / m_face->units_per_EM  ;
	double pixelAdjustX = scalefactor * ( ( double ) QApplication::desktop()->physicalDpiX() / 72.0 );
	double pixelAdjustY = scalefactor * ( ( double ) QApplication::desktop()->physicalDpiX() / 72.0 );
	double pWidth = lineWidth ;
	const double distance = 20;
	QList<RenderedGlyph> refGlyph = otf->procstring ( spec, set );
// 	qDebug() << "Get line "<<spec;
	delete otf;
	otf = 0;
// 	qDebug() << "Deleted OTF";
	if ( refGlyph.count() == 0 )
	{
		return 0;
	}
	QPointF pen ( origine );

	if ( m_rasterFreetype )
	{
		QList<QGraphicsPixmapItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromGindexPix ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			// Now, all is in the log!
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * pixelAdjustX;
				pWidth -= refGlyph[i].xadvance * pixelAdjustX ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * pixelAdjustY;
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * pixelAdjustX;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}

			/*************************************************/
			if ( record )
				pixList.append ( glyph );
			
			mayBeRemoved.append(glyph);
						
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * pixelAdjustX ) + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() * scalefactor  ,
			                pen.y() + ( refGlyph[i].yoffset * pixelAdjustY ) - glyph->data ( GLYPH_DATA_BITMAPTOP ).toInt() );
			/*************************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * pixelAdjustX ;
			else if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * pixelAdjustY ;
		}
	}
	else
	{
		QList<QGraphicsPathItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPathItem *glyph = itemFromGindex ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * scalefactor;
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * scalefactor;
				pWidth -=  refGlyph[i].yadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * scalefactor ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}

			/**********************************************/
			if ( record )
				glyphList.append ( glyph );
			
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * scalefactor ),
			                pen.y() + ( refGlyph[i].yoffset * scalefactor ) );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			/*******************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * scalefactor;
			if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * scalefactor;
		}
	}


	releaseFace();
	return retValue + 1;
}

/// Shaped line
double FontItem::renderLine ( QString script, QGraphicsScene * scene, QString spec, QPointF origine,double lineWidth, double fsize, bool record )
{
	qDebug()<<"Shaped("<< spec <<")";
	double retValue(0.0);
	if ( spec.isEmpty() )
		return 0;
	if ( !m_isOpenType )
		return 0;
	ensureFace();

	otf = new FMOtf ( m_face, 0x10000 );
	if ( !otf )
		return 0;
	
	FMShaperFactory *shaperfactory = 0;
	switch(m_shaperType)
	{
		case FMShaperFactory::FONTMATRIX : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
		break;
		case FMShaperFactory::HARFBUZZ : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::HARFBUZZ );
		break;
		case FMShaperFactory::ICU : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::ICU );
		break;
		case FMShaperFactory::M17N : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::M17N );
		break;
		case FMShaperFactory::PANGO : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::PANGO );
		break;
		case FMShaperFactory::OMEGA : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::OMEGA);
		break;
		default : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
	}
	
	GlyphList refGlyph ( shaperfactory->doShape( spec ) );
	delete shaperfactory;
			
	if ( record )
		sceneList.append ( scene );
	double sizz = fsize;
	double scalefactor = sizz / m_face->units_per_EM  ;
	double pixelAdjustX = scalefactor * ( ( double ) QApplication::desktop()->physicalDpiX() / 72.0 );
	double pixelAdjustY = scalefactor * ( ( double ) QApplication::desktop()->physicalDpiX() / 72.0 );
	double pWidth = lineWidth ;
	const double distance = 20;


// 	qDebug() << "Get line "<<spec;
	delete otf;
	otf = 0;
// 	qDebug() << "Deleted OTF";
	if ( refGlyph.count() == 0 )
	{
		return 0;
	}
	QPointF pen ( origine );

	if ( m_rasterFreetype )
	{
		QList<QGraphicsPixmapItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPixmapItem *glyph = itemFromGindexPix ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * pixelAdjustX;
				pWidth -= refGlyph[i].xadvance * pixelAdjustX ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * pixelAdjustY;
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * pixelAdjustX;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * pixelAdjustY ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPixmapItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							pixList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}

			/*************************************************/
			if ( record )
				pixList.append ( glyph );
			
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			glyph->setData ( GLYPH_DATA_FONTNAME , fancyName() );
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * pixelAdjustX ) + glyph->data ( GLYPH_DATA_BITMAPLEFT ).toDouble() * scalefactor  ,
			                pen.y() - ( refGlyph[i].yoffset * pixelAdjustY ) - glyph->data ( GLYPH_DATA_BITMAPTOP ).toInt() );
			/*************************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * pixelAdjustX ;
			else if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * pixelAdjustY ;
		}
	}
	else
	{
		QList<QGraphicsPathItem*> mayBeRemoved;
		for ( int i=0; i < refGlyph.count(); ++i )
		{
			QGraphicsPathItem *glyph = itemFromGindex ( refGlyph[i].glyph , sizz );
			if ( !glyph )
				continue;
			if( spec.at(refGlyph[i].log).category() == QChar::Separator_Space )
			{
				mayBeRemoved.clear();
			}

			if ( m_progression == PROGRESSION_RTL )
			{
				pen.rx() -= refGlyph[i].xadvance * scalefactor;
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_BTT )
			{
				pen.ry() -= refGlyph[i].yadvance * scalefactor;
				pWidth -=  refGlyph[i].yadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_LTR )
			{
				pWidth -= refGlyph[i].xadvance * scalefactor;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}
			else if ( m_progression == PROGRESSION_TTB )
			{
				pWidth -=  refGlyph[i].yadvance * scalefactor ;
				if ( pWidth < distance )
				{
					delete glyph;
					
					retValue -= mayBeRemoved.count() - 1;
					foreach(QGraphicsPathItem *rm, mayBeRemoved)
					{
						scene->removeItem( rm );
						if(record)
							glyphList.removeAll(rm);
						delete rm;
					}
					
					break;
				}
			}

			/**********************************************/
			if ( record )
				glyphList.append ( glyph );
			
			mayBeRemoved.append(glyph);
			
			if(renderReturnWidth)
				retValue += glyph->data ( GLYPH_DATA_HADVANCE ).toDouble() * scalefactor;
			else
				retValue = refGlyph[i].log;
			
			scene->addItem ( glyph );
			glyph->setPos ( pen.x() + ( refGlyph[i].xoffset * scalefactor ),
			                pen.y() - ( refGlyph[i].yoffset * scalefactor ) );
			glyph->setZValue ( 100.0 );
			glyph->setData ( GLYPH_DATA_GLYPH ,"glyph" );
			/*******************************************/

			if ( m_progression == PROGRESSION_LTR )
				pen.rx() += refGlyph[i].xadvance * scalefactor;
			if ( m_progression == PROGRESSION_TTB )
				pen.ry() += refGlyph[i].yadvance * scalefactor;
		}
	}


	releaseFace();
	return retValue + 1;
}

//deprecated
void FontItem::deRender ( QGraphicsScene *scene )
{
	QList<int> rem;
	for ( int i = 0; i < pixList.count(); ++i )
	{
		if ( pixList[i]->scene() == scene )
		{
			scene->removeItem ( pixList[i] );
			rem.append ( i );
		}
	}
	for ( int i = rem.count() - 1; i >= 0; --i )
		pixList.removeAt ( rem[i] );

}

void FontItem::deRenderAll()
{
// 	qDebug() << m_name  <<"::deRenderAll()";
// 	QSet<QGraphicsScene*> collectedScenes;
	for ( int i = 0; i < pixList.count(); ++i )
	{
		if ( pixList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			pixList[i]->scene()->removeItem ( pixList[i] );
			delete pixList[i];
		}
	}
	pixList.clear();
	for ( int i = 0; i < glyphList.count(); ++i )
	{
		if ( glyphList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			glyphList[i]->scene()->removeItem ( glyphList[i] );
			delete glyphList[i];
		}
	}
	glyphList.clear();
	for ( int i = 0; i < labList.count(); ++i )
	{
		if ( labList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			labList[i]->scene()->removeItem ( labList[i] );
			delete labList[i];
		}
	}
	labList.clear();
	for ( int i = 0; i < selList.count(); ++i )
	{
		if ( selList[i]->scene() )
		{
// 			collectedScenes.insert ( pixList[i]->scene() );
			selList[i]->scene()->removeItem ( selList[i] );
			delete selList[i];
		}
	}
	selList.clear();
	allIsRendered = false;
}

QByteArray FontItem::pixarray ( uchar * b, int len )
{
	uchar *imgdata =  b ;
	QByteArray buffer ( len * 4, 255 );
	QDataStream stream ( &buffer,QIODevice::WriteOnly );
	for ( int i = 0 ; i < len; ++i )
	{

		stream << ( quint8 ) ~imgdata[i];
		stream << ( quint8 ) ~imgdata[i];
		stream << ( quint8 ) ~imgdata[i];
		stream << ( quint8 ) imgdata[i];
	}

	return buffer;
}

//Render all is dangerous ;)
// We now render langs

int FontItem::countCoverage ( int begin_code, int end_code )
{
	ensureFace();
	FT_ULong  charcode = begin_code ;
	FT_UInt   gindex = 0;
	int count = 0;
	if ( begin_code >= 0 )
	{
// 		if ( hasUnicode )
		{
			while ( charcode <= end_code )
			{
				charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
				if ( !gindex )
					break;
				++count;
			}
		}
// 		else
// 		{
// 			while ( charcode <= end_code )
// 			{
// 				if ( charcode < m_numGlyphs )
// 				{
// 					++charcode;
// 					++count;
// 				}
// 				else
// 					break;
// 			}
// 		}
	}
	else
	{
		FT_UInt anIndex = 1;
		count = m_numGlyphs;
		FT_UInt anyChar =  FT_Get_First_Char ( m_face, &anIndex );
		while ( anIndex )
		{
			anyChar =  FT_Get_Next_Char ( m_face,anyChar,&anIndex );
			if ( anIndex )
				--count;
		}
	}
	releaseFace();
	return count - 1;//something weird with freetype which put a valid glyph at the beginning of each lang ??? Or a bug here...
}

void FontItem::renderAll ( QGraphicsScene * scene , int begin_code, int end_code )
{

	ensureFace();
// 	if ( allIsRendered )
// 		return;
	FMGlyphsView *allView = reinterpret_cast<FMGlyphsView*> ( scene->views() [0] );
	qDebug() <<"renderAll("<< begin_code<<","<<end_code <<")";
	deRenderAll();
	if ( !allView->isVisible() )
	{
		releaseFace();
		return;
	}


	adjustGlyphsPerRow ( allView->width() );
	QRectF exposedRect ( allView->visibleSceneRect() );

	double leftMargin = ( ( exposedRect.width() - ( 100 * m_glyphsPerRow ) ) / 2 ) + 30;
	double aestheticTopMargin = 12;
	QPointF pen ( leftMargin, 50  + aestheticTopMargin );

	int nl = 0;

	FT_ULong  charcode;
	FT_UInt   gindex = 1;
	double sizz = 50;
	charcode = begin_code;
	QPen selPen ( Qt::gray );
	QFont infoFont ( "Helvetica",8 );
	QBrush selBrush ( QColor ( 255,255,255,0 ) );
	if ( begin_code >= 0 )
	{
		if ( hasUnicode )
		{
			while ( charcode <= end_code && gindex )
			{
				if ( nl == m_glyphsPerRow )
				{
					nl = 0;
					pen.rx() = leftMargin;
					pen.ry() += 100;
				}
				if ( ( pen.y() + 100 ) < exposedRect.y() || pen.y() - 100 > ( exposedRect.y() + exposedRect.height() ) )
				{
					charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
// 					qDebug() << "charcode = "<< charcode <<" ; gindex = "<< gindex;
					pen.rx() += 100;
					++nl;

					continue;
				}

				QGraphicsPathItem *pitem = itemFromChar ( charcode , sizz );
				if ( pitem )
				{
					uint ucharcode = charcode;

					scene->addItem ( pitem );
					pitem->setPos ( pen );
					pitem->setData ( 1,"glyph" );
					pitem->setData ( 2,gindex );
					pitem->setData ( 3,ucharcode );
					glyphList.append ( pitem );

					pitem->setZValue ( 10 );

					QGraphicsTextItem *tit= scene->addText ( glyphName ( charcode ), infoFont );
					tit->setPos ( pen.x()-27,pen.y() + 15 );
					tit->setData ( 1,"label" );
					tit->setData ( 2,gindex );
					tit->setData ( 3,ucharcode );
					labList.append ( tit );
					tit->setZValue ( 1 );

					QGraphicsTextItem *tit2= scene->addText ( "U+" + QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) )  +" ("+ QString::number ( charcode ) +")"  , infoFont );
					tit2->setPos ( pen.x()-27,pen.y() + 28 );
					tit2->setData ( 1,"label" );
					tit2->setData ( 2,gindex );
					tit2->setData ( 3,ucharcode );
					labList.append ( tit2 );
					tit2->setZValue ( 1 );

					QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
					rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
					rit->setData ( 1,"select" );
					rit->setData ( 2,gindex );
					rit->setData ( 3,ucharcode );
					rit->setZValue ( 100 );
					selList.append ( rit );

					pen.rx() += 100;
					++nl;
				}
				charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
			}
		}
		else // Has not Unicode
		{
			// Here are fake charcodes (glyph index)
			while ( charcode <= end_code )
			{
				if ( nl == m_glyphsPerRow )
				{
					nl = 0;
					pen.rx() = leftMargin;
					pen.ry() += 100;
				}

				if ( ( pen.y() + 100 ) < exposedRect.y() || pen.y() - 100 > ( exposedRect.y() + exposedRect.height() ) )
				{
					++charcode;
					++nl;

					continue;
				}

				QGraphicsPathItem *pitem = itemFromGindex ( charcode , sizz );
				if ( pitem )
				{
					scene->addItem ( pitem );
					pitem->setPos ( pen );
					pitem->setData ( 1,"glyph" );
					pitem->setData ( 2,gindex );
					pitem->setData ( 3,0 );
					glyphList.append ( pitem );
					pitem->setZValue ( 10 );

					QGraphicsTextItem *tit= scene->addText ( QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) ) , infoFont );
					tit->setPos ( pen.x(),pen.y() + 15 );
					tit->setData ( 1,"label" );
					tit->setData ( 2,gindex );
					tit->setData ( 3,0 );
					labList.append ( tit );
					tit->setZValue ( 1 );

					QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
					rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
					rit->setData ( 1,"select" );
					rit->setData ( 2,gindex );
					rit->setData ( 3,0 );
					rit->setZValue ( 100 );
					selList.append ( rit );

					pen.rx() += 100;
					++nl;
				}
				else
				{
					break;
				}
				++charcode;
			}
		}
	}
	else // beginCode is negative - it means search for out charmap glyphs
	{
		// 1/ what is "out charmap"?
		FT_UInt anIndex = 1;
		QList<bool> notCovered;
		for ( int i=1; i  < m_numGlyphs +1; ++i )
			notCovered << true;
		FT_UInt anyChar =  FT_Get_First_Char ( m_face, &anIndex );
		while ( anIndex )
		{
			anyChar =  FT_Get_Next_Char ( m_face,anyChar,&anIndex );
			if ( anIndex )
				notCovered[anIndex] = false;
		}

		// 2/ fill with glyphs
		for ( int i = 1; i < notCovered.count(); ++i )
		{
			if ( !notCovered[i] )
				continue;
			if ( nl == m_glyphsPerRow )
			{
				nl = 0;
				pen.rx() = leftMargin;
				pen.ry() += 100;
			}

			if ( ( pen.y() + 100 ) < exposedRect.y() || pen.y() - 100 > ( exposedRect.y() + exposedRect.height() ) )
			{
				++nl;

				continue;
			}

			QGraphicsPathItem *pitem = itemFromGindex ( i , sizz );
			if ( pitem )
			{
				scene->addItem ( pitem );
				pitem->setPos ( pen );
				pitem->setData ( 1,"glyph" );
				pitem->setData ( 2, i );
				pitem->setData ( 3,0 );
				glyphList.append ( pitem );
				pitem->setZValue ( 10 );

				QGraphicsTextItem *tit= scene->addText ( QString ( "I+%1" ).arg ( i ), infoFont );
				tit->setPos ( pen.x(),pen.y() + 15 );
				tit->setData ( 1,"label" );
				tit->setData ( 2,i );
				tit->setData ( 3,0 );
				labList.append ( tit );
				tit->setZValue ( 1 );

				QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
				rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
				rit->setData ( 1,"select" );
				rit->setData ( 2,i );
				rit->setData ( 3,0 );
				rit->setZValue ( 100 );
				selList.append ( rit );

				pen.rx() += 100;
				++nl;
			}
		}
	}

	scene->setSceneRect ( QRectF ( 0,0, m_glyphsPerRow * 100 + 30, pen.y() + 100 ) );
	allIsRendered = true;
	releaseFace();

	exposedRect = allView->visibleSceneRect();
// 	qDebug() << "ENDOFRENDERALL" <<exposedRect.x() << exposedRect.y() << exposedRect.width() << exposedRect.height();
}

int FontItem::renderChart ( QGraphicsScene * scene, int begin_code, int end_code ,double pwidth, double pheight )
{
	qDebug() <<"FontItem::renderChart ("<< begin_code<<end_code <<")";

	ensureFace();
	int nl ( 0 );
	int retValue ( 0 );

	FT_ULong  charcode;
	FT_UInt   gindex = 1;
	double sizz = 50;
	charcode = begin_code;
	adjustGlyphsPerRow ( pwidth );

	double leftMargin = 30 + ( ( pwidth - ( m_glyphsPerRow * 100 ) ) / 2 );
	double aestheticTopMargin = 0;
	QPointF pen ( leftMargin, sizz + aestheticTopMargin );


	QPen selPen ( Qt::gray );
	QFont infoFont ( "Helvetica",8 );
	QBrush selBrush ( QColor ( 255,255,255,0 ) );

	while ( charcode <= end_code && gindex )
	{
		if ( nl == m_glyphsPerRow )
		{
			nl = 0;
			pen.rx() = leftMargin;
			pen.ry() += 100;
		}
		if ( pen.y() > pheight - 30 )
		{
			releaseFace();
			return  retValue  ;
		}


		QGraphicsPathItem *pitem = itemFromChar ( charcode , sizz );
		if ( pitem )
		{
			uint ucharcode = charcode;

			scene->addItem ( pitem );
			pitem->setPos ( pen );
			pitem->setData ( 1,"glyph" );
			pitem->setData ( 2,gindex );
			pitem->setData ( 3,ucharcode );
// 			glyphList.append ( pitem );

			pitem->setZValue ( 10 );

			QGraphicsTextItem *tit= scene->addText ( glyphName ( charcode ), infoFont );
			tit->setPos ( pen.x()-27,pen.y() + 15 );
			tit->setData ( 1,"label" );
			tit->setData ( 2,gindex );
			tit->setData ( 3,ucharcode );
// 			labList.append ( tit );
			tit->setZValue ( 1 );

			QGraphicsTextItem *tit2= scene->addText ( "U+" + QString ( "%1" ).arg ( charcode,4,16,QLatin1Char ( '0' ) )  +" ("+ QString::number ( charcode ) +")"  , infoFont );
			tit2->setPos ( pen.x()-27,pen.y() + 28 );
			tit2->setData ( 1,"label" );
			tit2->setData ( 2,gindex );
			tit2->setData ( 3,ucharcode );
// 			labList.append ( tit2 );
			tit2->setZValue ( 1 );

			QGraphicsRectItem *rit = scene->addRect ( pen.x() -30,pen.y() -50,100,100,selPen,selBrush );
			rit->setFlag ( QGraphicsItem::ItemIsSelectable,true );
			rit->setData ( 1,"select" );
			rit->setData ( 2,gindex );
			rit->setData ( 3,ucharcode );
			rit->setZValue ( 100 );

			pen.rx() += 100;
			++nl;
			++retValue;
		}
		retValue = charcode;
		charcode = FT_Get_Next_Char ( m_face, charcode, &gindex );
	}
	releaseFace();

	return retValue ;
}

QString FontItem::infoText ( bool fromcache )
{
// 	if ( !m_cacheInfo.isEmpty() && fromcache )
// 		return m_cacheInfo;

	bool rFace ( false );
	if ( moreInfo.isEmpty() /*|| !fromcache*/ )
	{
		ensureFace();
		rFace = true;
		if ( m_isOpenType == true /*testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" */ )
		{
			moreInfo_sfnt();
		}
		if ( m_path.endsWith ( ".pfb",Qt::CaseInsensitive ) )
		{
			moreInfo_type1();
		}
	}

	/**
	Selectors are :
	#headline
	#technote
	.infoblock
	.infoname
	.langmatch
	.langundefined
	.langnomatch
	*/
	QString ret;

	QString panStringOut;
	QString panBlockOut;
	if ( !m_panose.isEmpty() )
	{
// 		qDebug() <<"WE HAVE PANOSE \\o/";
		panStringOut = " | Panose: " + m_panose;
		//TODO output Panose as text (in panBlockOut)
		QMap<QString, QString>::const_iterator pat ( panoseInfo.constBegin() );
		for ( ;pat != panoseInfo.constEnd(); ++pat )
		{
			panBlockOut += "<div class=\"panose_name\">" + pat.key() + "</div>";
			panBlockOut += "<div class=\"panose_desc\">" + pat.value() + "</div>";
		}
	}
	else
	{
		qDebug() <<"ARGH"<< m_family << m_variant;
	}

	QMap<QString, QStringList> orderedInfo;
	ret += "<div id=\"headline\">" + fancyName() + "</div>\n" ;
	ret += "<div id=\"technote\">"+ QString::number ( m_numGlyphs ) + " glyphs | Type: "+ m_type +" | Charmaps: " + m_charsets.join ( ", " ) + panStringOut +"</div>";


// 	if ( !moreInfo.isEmpty() ) // moreInfo.isNotEmpty
	{
		QString sysLang = QLocale::languageToString ( QLocale::system ().language() ).toUpper();
		QString sysCountry = QLocale::countryToString ( QLocale::system ().country() ).toUpper();
		QString sysLoc = sysLang + "_"+ sysCountry;

		//We must iter once to find localized strings and ensure default ones are _not_ shown in these cases
		QStringList localizedKeys;
		for ( QMap<int, QMap<QString, QString> >::const_iterator lit = moreInfo.begin(); lit != moreInfo.end(); ++lit )
		{
			for ( QMap<QString, QString>::const_iterator mit = lit.value().begin(); mit != lit.value().end(); ++mit )
			{
				if ( langIdMap[ lit.key() ].contains ( sysLang ) )
				{
					localizedKeys << mit.key();
				}
			}
		}

		QString styleLangMatch;
		for ( QMap<int, QMap<QString, QString> >::const_iterator lit = moreInfo.begin(); lit != moreInfo.end(); ++lit )
		{
			if ( langIdMap[ lit.key() ].contains ( sysLang ) ) // lang match
			{
				styleLangMatch = "\"langmatch\"";
			}
			else if ( langIdMap[ lit.key() ] == "DEFAULT" ) // lang does not match but it’s international name
			{
				styleLangMatch = "\"langundefined\"";
			}
			else // lang does not match at all
			{
				styleLangMatch = "\"langnomatch\"";
			}
			for ( QMap<QString, QString>::const_iterator mit = lit.value().begin(); mit != lit.value().end(); ++mit )
			{
				if ( langIdMap[ lit.key() ].contains ( sysLang ) )
				{
					QString name_value = mit.value();
					name_value.replace ( QRegExp ( "(http://S+)" ), "<a href=\"\\1\">\\1</a>" );//Make HTTP links
					name_value.replace ( "\n","<br/>" );
					orderedInfo[ mit.key() ] << "<div class="+ styleLangMatch +">" + name_value +"</div>";
					if ( mit.key() == tr ( "Font Subfamily" ) )
						m_variant = mit.value();
				}
				else if ( langIdMap[ lit.key() ] == "DEFAULT" && !localizedKeys.contains ( mit.key() ) )
				{
					QString name_value = mit.value();
					name_value.replace ( QRegExp ( "(http://.+)\\s*" ), "<a href=\"\\1\">\\1</a>" );//Make HTTP links
					name_value.replace ( "\n","<br/>" );
					orderedInfo[ mit.key() ] << "<div class="+ styleLangMatch +">" + name_value +"</div>";
					if ( mit.key() == tr ( "Font Subfamily" ) )
						m_variant = mit.value();
				}
			}
		}
	}



	for ( int i = 1; i < name_meaning.count(); ++i )
	{
		if ( orderedInfo.contains ( name_meaning[i] ) )
		{
// 			qDebug() << orderedInfo[name_meaning[i]].join("|");
			QStringList entries ( orderedInfo[name_meaning[i]].toSet().toList() );
			ret += "<div class=\"infoblock\"><div class=\"infoname\">" + name_meaning[i]+ "</div>" + entries.join ( " " ) +"</div>";
		}
		if ( i == 7 ) //trademark
			i = -1;
		if ( i == 0 ) //Copyright
			i = 7;
	}

	ret += "<div id=\"panose_block\">" + panBlockOut + "</div>";
// 	m_cacheInfo = ret;

	if ( rFace )
		releaseFace();

	return ret;
}

QString FontItem::glyphName ( int codepoint )
{
	ensureFace();

	int index = FT_Get_Char_Index ( m_face, codepoint );
	if ( index== 0 )
	{
		return "noname";
	}

	QByteArray key ( 1001,0 );
	if ( FT_HAS_GLYPH_NAMES ( m_face ) )
	{
		FT_Get_Glyph_Name ( m_face, index, key.data() , 1000 );
		if ( key[0] == char ( 0 ) )
		{
			key = "noname";
		}
	}
	else
	{
		key = "noname";
	}
	return QString ( key );
	releaseFace();
}


QString FontItem::infoGlyph ( int index, int code )
{
	ensureFace();
	QString ret;
	ret += glyphName ( code ) ;
	ret += ", " + QObject::tr ( "codepoint is U+" ) ;
	ret += QString ( "%1" ).arg ( code, 4, 16, QChar ( 0x0030 ) ) ;
	ret += " (int"+ QString::number ( code ) +")";

	releaseFace();
	return ret;
}

//deprecated
QString FontItem::toElement()
{
	QString ret;
	ret = "<fontfile><file>%1</file><tag>%2</tag></fontfile>";
	return ret.arg ( name() ).arg ( tags().join ( "</tag><tag>" ) );
}

QGraphicsPathItem * FontItem::hasCodepoint ( int code )
{
	for ( int i=0;i< glyphList.count();++i )
	{
		if ( glyphList.at ( i )->data ( 3 ).toInt() == code )
			return glyphList.at ( i );
	}
	return 0;
}


QPixmap FontItem::oneLinePreviewPixmap ( QString oneline , QColor bg_color, int size_w )
{
	if ( m_remote )
		return fixedPixmap;
	if ( !theOneLinePreviewPixmap.isNull() )
	{
		if ( theOneLinePreviewPixmap.width() == size_w )
			return theOneLinePreviewPixmap;
	}
	if ( !ensureFace() )
		return QPixmap();
	QRectF savedRect = theOneLineScene->sceneRect();

	double theSize = typotek::getInstance()->getPreviewSize();
	double pt2px = QApplication::desktop()->physicalDpiX() / 72.0;
	double theHeight = theSize * 1.3 * pt2px;
	double theWidth;
	if ( size_w == 0 )
	{
		theWidth = theSize * pt2px * oneline.count() * 1.2;
	}
	else
	{
		theWidth = size_w;
	}
// 	qDebug() << theSize << theHeight << theWidth;
	theOneLineScene->setSceneRect ( 0,0,theWidth, theHeight );
	bool pRTL = typotek::getInstance()->getPreviewRTL();
	QPoint pen ( pRTL ? theWidth - 20 : 20 , theSize *  pt2px );

	double fsize = theSize ;
	double scalefactor = theSize / m_face->units_per_EM;

	QPixmap linePixmap ( theWidth,theHeight );
	linePixmap.fill ( bg_color );
	QPainter apainter ( &linePixmap );
	QVector<QRgb> palette;
	int notRenderedGlyphsCount ( 0 );

	for ( int i =0;i < oneline.count() ; ++i )
	{
		int glyphIndex = FT_Get_Char_Index ( m_face, oneline[i].unicode() );
		if ( glyphIndex == 0 )
		{
			++notRenderedGlyphsCount;
			continue;
		}

		FT_Set_Char_Size ( m_face,
		                   fsize  * 64 ,
		                   0,
		                   QApplication::desktop()->physicalDpiX(),
		                   QApplication::desktop()->physicalDpiY() );

		ft_error = FT_Load_Glyph ( m_face, glyphIndex, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING );
		if ( ft_error )
		{
			continue;
		}

		ft_error = FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL );
		if ( ft_error )
		{
			continue;
		}

		if ( pRTL )
			pen.rx() -= qRound ( m_glyph->linearHoriAdvance / 65536 );

		apainter.drawImage ( pen.x() +  m_glyph->bitmap_left , pen.y() - m_glyph->bitmap_top , glyphImage() );

		if ( !pRTL )
			pen.rx() += qRound ( m_glyph->linearHoriAdvance / 65536 );

	}
	/// Check if we have drawn something
	if ( notRenderedGlyphsCount == oneline.count() )
	{
		pen.rx() = 0; // we don’t bother about RTL here.
		//If not we draw first available characters.
		FT_ULong  charCode;
		FT_UInt   glyphIndex;
		charCode = FT_Get_First_Char ( m_face, &glyphIndex );
		for ( int i =0;i < oneline.count() ; ++i ) // get same number of glyphs than normal preview word
		{
			if ( glyphIndex == 0 )
			{
				continue;
			}
			ft_error = FT_Load_Glyph ( m_face, glyphIndex, FT_LOAD_NO_SCALE );
			if ( ft_error )
			{
				continue;
			}
			double advance = m_glyph->metrics.horiAdvance  * scalefactor * pt2px;
			double leftBearing = ( double ) m_glyph->metrics.horiBearingX * scalefactor * pt2px;
// 			qDebug() << oneline[i] <<  m_glyph->metrics.horiAdvance  << advance ;
			FT_Set_Char_Size ( m_face,
			                   fsize  * 64 ,
			                   0,
			                   QApplication::desktop()->physicalDpiX(),
			                   QApplication::desktop()->physicalDpiY() );
			ft_error = FT_Load_Glyph ( m_face, glyphIndex, FT_LOAD_DEFAULT );
			if ( ft_error )
			{
				continue;
			}
			ft_error = FT_Render_Glyph ( m_face->glyph, FT_RENDER_MODE_NORMAL );
			if ( ft_error )
			{
				continue;
			}

			pen.ry() = ( theSize * pt2px ) - m_glyph->bitmap_top;

			apainter.drawImage ( pen.x() + leftBearing, pen.y(), glyphImage() );

			pen.rx() +=  advance;

			charCode = FT_Get_Next_Char ( m_face, charCode, &glyphIndex );

		}
	}
	apainter.end();
	releaseFace();

	theOneLinePreviewPixmap = linePixmap;


	if ( !theOneLinePreviewPixmap.isNull() )
		return theOneLinePreviewPixmap;

	theOneLinePreviewPixmap = QPixmap ( theWidth,theHeight );
	theOneLinePreviewPixmap.fill ( Qt::lightGray );
	return theOneLinePreviewPixmap;
}

void FontItem::clearPreview()
{
	if ( m_remote )
		return;
	if ( !theOneLinePreviewPixmap.isNull() )
		theOneLinePreviewPixmap = QPixmap();
}



/** reminder
FT_SfntName::name_id
Code  	Meaning
0 	Copyright
1 	Font Family
2 	Font Subfamily
3 	Unique font identifier
4 	Full font name
5 	Version string
6 	Postscript name for the font
7 	Trademark
8 	Manufacturer Name.
9 	Designer
10 	Description
11 	URL Vendor
12 	URL Designer
13 	License Description
14 	License Info URL
15 	Reserved; Set to zero.
16 	Preferred Family
17 	Preferred Subfamily
18 	Compatible Full (Macintosh only)
19 	Sample text
20 	PostScript CID findfont name
*/
void FontItem::moreInfo_sfnt()
{
	if ( !ensureFace() )
		return;

	FT_SfntName tname;

	if ( name_meaning.isEmpty() )
	{
		fillNamesMeaning();
	}
	int tname_count = FT_Get_Sfnt_Name_Count ( m_face );


	//TODO check encodings and platforms
	for ( int i=0; i < tname_count; ++i )
	{
		FT_Get_Sfnt_Name ( m_face,i,&tname );
		if ( tname.language_id != 0 && tname.language_id != theLocalLangCode )
		{
			continue;
		}
		QString akey;
		if ( tname.name_id >  255 )
		{
// 			qDebug() << name() <<" has vendor’s specific name id ->" << tname.name_id;
			if ( tname.string_len > 0 )
			{
				akey = "VendorKey_" + QString::number ( tname.name_id );
			}
			else
			{
				continue;
			}

		}
		else if ( tname.name_id <= name_meaning.count() )
		{
			akey = name_meaning.at ( tname.name_id );
		}
		else
		{
// 			qDebug() << name() <<" : It seems there are new name IDs in TT spec ("<< tname.name_id <<")!";
			continue;
		}

		QString avalue;
		///New plan, we’ll put here _user contributed_ statements!
		if ( tname.platform_id ==TT_PLATFORM_MICROSOFT && tname.encoding_id == TT_MS_ID_UNICODE_CS ) // Corresponds to a Microsoft WGL4 charmap, matching Unicode.
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "UTF-16BE" );
			avalue = codec->toUnicode ( array );
		}
		else if ( tname.platform_id ==TT_PLATFORM_MICROSOFT && tname.encoding_id == TT_MS_ID_SYMBOL_CS ) // Corresponds to Microsoft symbol encoding. PM - don(t understand what it does here? seen in StandardSym.ttf
		{
			avalue = "Here, imagine some nice symbols!";
		}
		else if ( tname.platform_id == TT_PLATFORM_MACINTOSH  && tname.encoding_id == TT_APPLE_ID_DEFAULT ) // Unicode version 1.0
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "ISO 8859-15" ); // ### give better result than UTF ???
			avalue = codec->toUnicode ( array );
		}
		else if ( tname.platform_id == TT_PLATFORM_APPLE_UNICODE  && tname.encoding_id == TT_APPLE_ID_DEFAULT ) // Unicode version 1.0
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "ISO 8859-15" ); // ### give better result than UTF ???
			avalue = codec->toUnicode ( array );
		}
		// from  Pajarico, pajarico chez gmail point com
		else if ( tname.platform_id == TT_PLATFORM_APPLE_UNICODE  && tname.encoding_id == TT_APPLE_ID_UNICODE_2_0 )
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "UTF-16" );
			avalue = codec->toUnicode ( array );
		}
		else if ( tname.platform_id == TT_PLATFORM_MACINTOSH   /*&& tname.encoding_id == TT_MAC_ID_TRADITIONAL_CHINESE*/ )
		{
			QByteArray array ( ( const char* ) tname.string, tname.string_len );
			QTextCodec *codec = QTextCodec::codecForName ( "Apple Roman" );
			avalue = codec->toUnicode ( array );
		}
		else
		{
			avalue = "Unexpected platform - encoding pair ("
			         + QString::number ( tname.platform_id )
			         + "," + QString::number ( tname.encoding_id )
			         + ")\nPlease contact Fontmatrix team.\nRun Fontmatrix in console to see more info.\nPlease, if possible, provide a font file to test.";

			qDebug() << m_name
			<< "platform_id("
			<< tname.platform_id
			<<") - encoding_id("
			<< tname.encoding_id
			<<") - "
			<< QString::number ( tname.language_id )
			<< langIdMap[tname.language_id];
		}


		if ( !avalue.isEmpty() )
		{
			moreInfo[tname.language_id][akey] = avalue;
		}
	}

	// Is there an OS/2 table?
	TT_OS2 *os2 = static_cast<TT_OS2*> ( FT_Get_Sfnt_Table ( m_face, ft_sfnt_os2 ) );
	if ( os2 /* and  wantAutoTag*/ )
	{
		// Just want the panose data
		m_panose.clear();
		for ( int bI ( 0 ); bI < 10; ++bI )
		{
			m_panose += QString::number ( os2->panose[bI] ) ;
			panoseInfo[ panoseKeys[bI] ] = panoseMap[ panoseKeys[bI] ][ os2->panose[bI] ] ;
// 			if(os2->panose[bI] > 1 ) // "any" nor "not fit" does not seem rather interesting tags :)
// 			{
// 				QString pTag( panoseKeys[bI] +" - "+ panoseMap[ panoseKeys[bI] ][ os2->panose[bI] ]);
// 				if(!m_tags.contains(pTag) )
// 				{
// 					m_tags << pTag;
// 					if(!typotek::tagsList.contains(pTag))
// 					{
// 						typotek::tagsList << pTag;
// 					}
// 				}
// 			}
		}
// 		moreInfo[0]["Panose"] = pan;


	}

	releaseFace();
}

QString FontItem::getAlternateFamilyName()
{
	if ( !ensureFace() )
		return QString();

	FT_SfntName tname;
	int tname_count = FT_Get_Sfnt_Name_Count ( m_face );
	for ( int i=0; i < tname_count; ++i )
	{
		FT_Get_Sfnt_Name ( m_face , i , &tname );
		if ( tname.name_id == 1 && tname.language_id == 0 )
		{
			return QString ( QByteArray ( ( const char* ) tname.string, tname.string_len ) );
		}

	}
	
	releaseFace();
	return QString();
}

QString FontItem::getAlternateVariantName()
{
	if ( !ensureFace() )
		return QString();

	FT_SfntName tname;
	int tname_count = FT_Get_Sfnt_Name_Count ( m_face );
	for ( int i=0; i < tname_count; ++i )
	{
		FT_Get_Sfnt_Name ( m_face , i , &tname );
		if ( tname.name_id == 2 && tname.language_id == 0 )
		{
			return QString ( QByteArray ( ( const char* ) tname.string, tname.string_len ) );
		}

	}

	releaseFace();
	return QString();
}


void FontItem::moreInfo_type1()
{
	if ( !ensureFace() )
		return;

	PS_FontInfoRec sinfo ;
	int err = FT_Get_PS_Font_Info ( m_face,&sinfo );
	if ( err )
	{
		qDebug() <<"FT_Get_PS_Font_Info("<< m_name <<")"<<" failed :" << err;
		return;
	}
	// full_name version notice
	moreInfo[0][tr ( "Full font name" ) ] = sinfo.full_name;
	moreInfo[0][tr ( "Version string" ) ] = sinfo.version;
	moreInfo[0][tr ( "Description" ) ] = sinfo.notice;

	releaseFace();
}


void FontItem::setTags ( QStringList l )
{
	bool act = isActivated();
	m_tags = l;
	setActivated ( act );
}

/// When glyphsView is resized we wantto adjust the number of columns
void FontItem::adjustGlyphsPerRow ( int width )
{
	m_glyphsPerRow = 1;
	int extraAdjust = 30;
	for ( int i = 1; i < 30 ; ++i )
	{
		if ( ( i*100 ) +extraAdjust > width )
			return;
		else
			m_glyphsPerRow = i;
	}
}

bool FontItem::isActivated()
{
	if ( m_tags.contains ( "Activated_Off" ) )
		return false;
	else if ( m_tags.contains ( "Activated_On" ) )
		return true;
	else
		m_tags << "Activated_Off";
	return false;
}

void FontItem::setActivated ( bool act )
{
	if ( act )
	{
		if ( isActivated() )
		{
			return;
		}
		else
		{
			m_tags.removeAll ( "Activated_Off" );
			m_tags << "Activated_On";
		}
	}
	else
	{
		if ( !isActivated() )
		{
			return;
		}
		else
		{
			m_tags.removeAll ( "Activated_On" );
			m_tags << "Activated_Off";
		}
	}
}



FMOtf * FontItem::takeOTFInstance()
{
	ensureFace();
	if ( m_isOpenType )
		otf = new FMOtf ( m_face );
	return otf;

	// It is a case where we don’t release face, thr caller have to call releaseOTFInstance;
}

void FontItem::releaseOTFInstance ( FMOtf * rotf )
{
	if ( rotf == otf )
	{
		delete otf;
		otf = 0;
	}
	releaseFace();
}

int FontItem::showFancyGlyph ( QGraphicsView *view, int charcode , bool charcodeIsAGlyphIndex )
{
	ensureFace();

	int ref ( fancyGlyphs.count() );
	QRect allRect ( view->rect() );
	QRect targetRect ( view->mapToScene ( allRect.topLeft() ).toPoint(),  view->mapToScene ( allRect.topRight() ) .toPoint() ) ;
	qDebug() <<  allRect.topLeft() << view->mapToScene ( allRect.topLeft() );

	// We’ll try to have a square subRect that fit in view ;-)
	int squareSideUnit = qMin ( allRect.width() ,  allRect.height() ) * 0.1;
	int squareSide = 8 * squareSideUnit;
	int squareXOffset = ( allRect.width() - squareSide ) / 2;
	int squareYOffset = ( allRect.height() - squareSide ) / 2;
	QRect subRect ( QPoint ( squareXOffset , squareYOffset ),
	                QSize ( squareSide, squareSide ) );
	QPixmap pix ( allRect.width(), allRect.height() );
	pix.fill ( QColor ( 30,0,0,120 ) );
	QPainter painter ( &pix );
	painter.setBrush ( Qt::white );
	painter.setPen ( QPen ( QColor ( 0,0,255,120 ) ) );
	painter.drawRoundRect ( subRect,5,5 );

	ft_error = FT_Set_Pixel_Sizes ( m_face, 0, subRect.height() * 0.8 );
	if ( ft_error )
	{
		return -1;
	}
	if ( !charcodeIsAGlyphIndex )
		ft_error = FT_Load_Char ( m_face, charcode  , FT_LOAD_RENDER );
	else
		ft_error = FT_Load_Glyph ( m_face, charcode  , FT_LOAD_RENDER );
	if ( ft_error )
	{
		return -1;
	}

	QVector<QRgb> palette;
	for ( int i = 0; i < m_face->glyph->bitmap.num_grays; ++i )
	{
		palette << qRgb ( 255 - i,255 - i,255 - i );
	}
	QImage img ( m_face->glyph->bitmap.buffer,
	             m_face->glyph->bitmap.width,
	             m_face->glyph->bitmap.rows,
	             m_face->glyph->bitmap.pitch,
	             QImage::Format_Indexed8 );
	img.setColorTable ( palette );

	double scaledBy = 1.0;
	if ( img.width() > subRect.width() )
	{
		scaledBy = ( double ) subRect.width() / ( double ) img.width() * 0.8;
		qDebug() <<"scaledBy = " << scaledBy ;
		img = img.scaledToWidth ( subRect.width() * 0.8,Qt::SmoothTransformation );

	}

	QPoint gPos ( subRect.topLeft() );
	gPos.rx() += ( subRect.width() - img.width() ) / 2;
	gPos.ry() += ( subRect.height() - img.height() ) /2;
	painter.drawImage ( gPos, img );


	// Draw metrics
	QPoint pPos ( gPos );
	pPos.rx() -= m_face->glyph->bitmap_left * scaledBy;
	pPos.ry() += m_face->glyph->bitmap_top * scaledBy;
	//left
	painter.drawLine ( pPos.x() , 0,
	                   pPos.x() , allRect.bottom() );
	//right
	painter.drawLine ( pPos.x() + m_face->glyph->metrics.horiAdvance / 64 * scaledBy , 0,
	                   pPos.x() + m_face->glyph->metrics.horiAdvance / 64 * scaledBy , allRect.bottom() );
	//baseline
	painter.drawLine ( 0, pPos.y() ,
	                   allRect.right(),  pPos.y() );

	painter.end();

	QGraphicsPixmapItem *fancyGlyph = new  QGraphicsPixmapItem;
	fancyGlyph->setPixmap ( pix );
	fancyGlyph->setZValue ( 10000 );
	fancyGlyph->setPos ( targetRect.topLeft() );
	view->scene()->addItem ( fancyGlyph );
	fancyGlyphs[ref] =  fancyGlyph ;


	QGraphicsTextItem *textIt = new QGraphicsTextItem;
	textIt->setTextWidth ( squareSide );

	QString itemNameStyle ( "background-color:#333;color:white;font-weight:bold;font-size:10pt" );
	QString itemValueStyle ( "background-color:#eee;font-style:italic;font-size:10pt" );
	if ( charcodeIsAGlyphIndex )
	{
		QString html;

		html = "<span style=\""+ itemNameStyle +"\"> Index </span>";
		html += "<span style=\""+ itemValueStyle +"\"> "+ QString::number ( charcode ) +" </span>";
		textIt->setHtml ( html );
	}
	else
	{
		QString html;

		QString catString;
		int cat ( QChar::category ( static_cast<uint> ( charcode ) ) );
		if ( cat == QChar::Mark_NonSpacing ) catString = QObject::tr ( "Mark, NonSpacing" );
		else if ( cat == QChar::Mark_SpacingCombining ) catString = QObject::tr ( "Mark, SpacingCombining" );
		else if ( cat == QChar::Mark_Enclosing ) catString = QObject::tr ( "Mark, Enclosing" );
		else if ( cat == QChar::Number_DecimalDigit ) catString = QObject::tr ( "Number, DecimalDigit" );
		else if ( cat == QChar::Number_Letter ) catString = QObject::tr ( "Number, Letter" );
		else if ( cat == QChar::Number_Other ) catString = QObject::tr ( "Number, Other" );
		else if ( cat == QChar::Separator_Space ) catString = QObject::tr ( "Separator, Space" );
		else if ( cat == QChar::Separator_Line ) catString = QObject::tr ( "Separator, Line" );
		else if ( cat == QChar::Separator_Paragraph ) catString = QObject::tr ( "Separator, Paragraph" );
		else if ( cat == QChar::Other_Control ) catString = QObject::tr ( "Other, Control" );
		else if ( cat == QChar::Other_Format ) catString = QObject::tr ( "Other, Format" );
		else if ( cat == QChar::Other_Surrogate ) catString = QObject::tr ( "Other, Surrogate" );
		else if ( cat == QChar::Other_PrivateUse ) catString = QObject::tr ( "Other, PrivateUse" );
		else if ( cat == QChar::Other_NotAssigned ) catString = QObject::tr ( "Other, NotAssigned" );
		else if ( cat == QChar::Letter_Uppercase ) catString = QObject::tr ( "Letter, Uppercase" );
		else if ( cat == QChar::Letter_Lowercase ) catString = QObject::tr ( "Letter, Lowercase" );
		else if ( cat == QChar::Letter_Titlecase ) catString = QObject::tr ( "Letter, Titlecase" );
		else if ( cat == QChar::Letter_Modifier ) catString = QObject::tr ( "Letter, Modifier" );
		else if ( cat == QChar::Letter_Other ) catString = QObject::tr ( "Letter, Other" );
		else if ( cat == QChar::Punctuation_Connector ) catString = QObject::tr ( "Punctuation, Connector" );
		else if ( cat == QChar::Punctuation_Dash ) catString = QObject::tr ( "Punctuation, Dash" );
		else if ( cat == QChar::Punctuation_Open ) catString = QObject::tr ( "Punctuation, Open" );
		else if ( cat == QChar::Punctuation_Close ) catString = QObject::tr ( "Punctuation, Close" );
		else if ( cat == QChar::Punctuation_InitialQuote ) catString = QObject::tr ( "Punctuation, InitialQuote" );
		else if ( cat == QChar::Punctuation_FinalQuote ) catString = QObject::tr ( "Punctuation, FinalQuote" );
		else if ( cat == QChar::Punctuation_Other ) catString = QObject::tr ( "Punctuation, Other" );
		else if ( cat == QChar::Symbol_Math ) catString = QObject::tr ( "Symbol, Math" );
		else if ( cat == QChar::Symbol_Currency ) catString = QObject::tr ( "Symbol, Currency" );
		else if ( cat == QChar::Symbol_Modifier ) catString = QObject::tr ( "Symbol, Modifier" );
		else if ( cat == QChar::Symbol_Other ) catString = QObject::tr ( "Symbol, Other" );

		html = "<span style=\""+ itemNameStyle +"\"> "+ tr ( "Category" ) + " </span>";
		html += "<span style=\""+ itemValueStyle +"\"> "+ catString +" </span>";

		textIt->setHtml ( html );
	}

// 	qDebug()<< textIt->toHtml();
	QPointF tPos ( subRect.left() + 20.0 , subRect.bottom() );
	textIt->setPos ( view->mapToScene ( tPos.toPoint() ) );
	textIt->setZValue ( 2000000 );
	textIt->setEnabled ( true );
	textIt->setFlags ( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable );
	textIt->setData ( 10, "FancyText" );
	view->scene()->addItem ( textIt );
	fancyTexts[ref] = textIt ;


	// Alternates
	if ( !charcodeIsAGlyphIndex )
	{
		QList<int> alts ( getAlternates ( charcode ) );
		qDebug() << "PALTS"<<alts;
		double altSize ( squareSide / 6 );
		double altXOffset ( subRect.top() + 10 );
		for ( int a ( 0 ); a < alts.count() ; ++a )
		{
			QGraphicsPixmapItem *gpi ( itemFromGindexPix ( alts.at ( a ), altSize ) );
			fancyAlternates[ref] << gpi;

			QImage altI ( gpi->pixmap().toImage().alphaChannel() );
			QPixmap altP ( altI.width() * 2, altI.height() * 2 );
			altP.fill ( Qt::transparent );
			QPainter altPainter ( &altP );
			altPainter.setRenderHint ( QPainter::Antialiasing,true );
			altPainter.setBrush ( Qt::black );
			altPainter.drawRoundRect ( 5,
			                           5,
			                           altP.width() - 10,
			                           altP.height()  - 10,
			                           20,20 );
			altPainter.drawImage ( altI.width() / 2.0, altI.height() / 2.0 , altI );

			gpi->setPixmap ( altP );


			view->scene()->addItem ( gpi );
			gpi->setPos ( view->mapToScene ( subRect.right()  ,altXOffset ) );
			altXOffset += altP.height();
			gpi->setZValue ( 9999999 );
			qDebug() <<gpi->pos() <<gpi->scenePos();
		}

	}

	releaseFace();
	return ref;

}

void FontItem::hideFancyGlyph ( int ref )
{
	if ( fancyGlyphs.contains ( ref ) )
	{
		QGraphicsPixmapItem *it = fancyGlyphs.value ( ref );
		it->scene()->removeItem ( it );
		fancyGlyphs.remove ( ref );
		delete it;

	}
	if ( fancyTexts.contains ( ref ) )
	{
		QGraphicsTextItem *it = fancyTexts.value ( ref );
		it->scene()->removeItem ( it );
		fancyTexts.remove ( ref );
		delete it;
	}
	if ( fancyAlternates.value ( ref ).count() )
	{
		QList<QGraphicsPixmapItem*> pil ( fancyAlternates.value ( ref ) );
		for ( int pidx ( 0 ); pidx < pil.count(); ++pidx )
		{
			QGraphicsPixmapItem *it = pil.at ( pidx ) ;
			it->scene()->removeItem ( it );
			delete it;
		}
		fancyAlternates.remove ( ref );
	}
}


bool FontItem::isLocal()
{
	QString shem = m_url.scheme();
	if ( shem.isEmpty() || shem == "file" )
		return true;
	return false;
}

/// We don’t want to download fonts yet. We just want something to fill font tree
void FontItem::fileRemote ( QString f , QString v, QString t, QString i, QPixmap p )
{
	m_family = f;
	m_variant = v;
	m_type = t;
	m_cacheInfo = i;
	fixedPixmap = p;
}

/// the same, but just for speedup startup with a lot of font files
void FontItem::fileLocal ( QString f, QString v, QString t, QString p, QMap<int, QMap< QString, QString > > i )
{
	m_family = f;
	m_variant = v;
	m_type = t;

	moreInfo = i;
}

void FontItem::fileLocal ( FontLocalInfo fli )
{
	m_family = fli.family;
	m_variant = fli.variant;
	m_type = fli.type;
	moreInfo = fli.info;
	if ( !fli.panose.isEmpty() )
	{
		m_panose = fli.panose;
		for ( int bI ( 0 ); bI < 10; ++bI )
		{
			panoseInfo[ panoseKeys[bI] ] = panoseMap.value ( panoseKeys[bI] ).value ( m_panose.mid ( bI,1 ).toInt() ) ;

		}

	}

}


/// Finally, we have to download the font file
int FontItem::getFromNetwork()
{
	qDebug() <<"FontItem::getFromNetwork()";
	if ( remoteCached )
		return 1;
	if ( stopperDownload )
		return 2;
	else
		stopperDownload = true;

	QUrl url ( m_path );
	remoteHerePath = typotek::getInstance()->remoteTmpDir() + QDir::separator() + QFileInfo ( url.path() ).fileName();

	rFile = new QFile ( remoteHerePath );
	if ( !rFile->open ( QIODevice::WriteOnly ) )
	{
		qDebug() << "Can’t open " << remoteHerePath;
		delete rFile;
// 		return false;
	}

	rHttp = new QHttp ( url.host() );
	qDebug() << "Init progress Dialog";
	rProgressDialog = new QProgressDialog ( typotek::getInstance() );
	rProgressDialog->setWindowTitle ( tr ( "Fontmatrix - Download" ) );
	rProgressDialog->setLabelText ( tr ( "Downloading %1." ).arg ( m_path ) );
	rProgressDialog->show();
	rProgressDialog->raise();
	rProgressDialog->activateWindow();
	qDebug() <<"Progress dialog done";

	connect ( rHttp,SIGNAL ( dataReadProgress ( int, int ) ),this,SLOT ( slotDowloadProgress ( int,int ) ) );
	connect ( rHttp,SIGNAL ( requestFinished ( int, bool ) ),this,SLOT ( slotDownloadEnd ( int, bool ) ) );
	connect ( rHttp,SIGNAL ( done ( bool ) ),this,SLOT ( slotDownloadDone ( bool ) ) );
	connect ( rHttp,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotDownloadState ( int ) ) );

	remoteId = rHttp->get ( url.path() , rFile );
	return 2;
}

void FontItem::slotDownloadStart ( int id )
{
// 	rProgressDialog->show();
	if ( id != remoteId )
	{
		qDebug() << "catched a weird request : " << id;
	}
}

void FontItem::slotDowloadProgress ( int done, int total )
{
	rProgressDialog->setMaximum ( total );
	rProgressDialog->setValue ( done );
	qDebug() << " [" <<done << "/"<< total<<"]" ;
}

void FontItem::slotDownloadEnd ( int id, bool error )
{
	qDebug() << m_path << "::slotDownloadEnd ["<< id <<"] when remoteCached = "<< remoteCached;
	if ( id != remoteId )
	{
		qDebug() << "WTF this id("<< id <<") comes from nowhere, our is "<< remoteId;
		return;
	}
	if ( remoteCached )
	{
		qDebug() << "Youre a bit late dude.";
		return;
	}
	else
	{
		remoteCached = true;
	}
	rFile->flush();
	rFile->close();
	rHttp->close();

	delete rProgressDialog;
	delete rFile;

	emit dowloadFinished();
}

void FontItem::slotDownloadDone ( bool error )
{
	qDebug() << "slotDownloadDone(" <<error<<")";
}

void FontItem::slotDownloadState ( int state )
{
// 	qDebug() << "slotDownloadState("<<state<<")";
	if ( state == QHttp::Unconnected  && rHttp )
	{
		qDebug() << "slotDownloadState( QHttp::Unconnected )";
		delete rHttp;
		rHttp = 0;
	}

}


void FontItem::trimSpacesIndex()
{
	if ( !spaceIndex.isEmpty() )
		return;
	if ( !ensureFace() )
		return;

	int gIndex ( 0 );
	for ( int i ( 0 ); i < legitimateNonPathChars.count(); ++i )
	{
		gIndex =   FT_Get_Char_Index ( m_face , legitimateNonPathChars[i] );
		if ( gIndex )
		{
// 			qDebug()<<"Space : " << legitimateNonPathChars[i] << " is : "<<gIndex;
			spaceIndex << gIndex;
		}
	}

	releaseFace();
}

QString FontItem::activationName()
{
	if ( m_remote || m_lock )
		return QString();

	QFileInfo fi ( m_path );
	QString prefix ( "%1-" );
	return  prefix.arg ( fi.size() ) + fi.fileName();
}

QString FontItem::activationAFMName()
{
	if ( m_remote || m_lock )
		return QString();
	if ( m_afm.isEmpty() )
		return QString();

	QFileInfo afi ( m_afm );
	QFileInfo fi ( m_path );
	QString prefix ( "%1-" );
	return  prefix.arg ( fi.size() ) + afi.fileName();
}

QList< int > FontItem::getAlternates ( int ccode )
{
	QList<int> ret;
	if ( !ensureFace() )
		return ret;
	if ( !otf && m_isOpenType )
	{
		otf = new FMOtf ( m_face );
		if ( !otf )
			return ret;
	}

	int glyphIndex ( FT_Get_Char_Index ( m_face, ccode ) );
	QList<OTFSet> setList;
	setList.clear();

	otf->set_table ( "GSUB" );
	foreach ( QString script, otf->get_scripts() )
	{
		otf->set_script ( script );
		foreach ( QString lang, otf->get_langs() )
		{
			otf->set_lang ( lang );
			QStringList fl ( otf->get_features() );
			if ( fl.contains ( "aalt" ) )
			{
				OTFSet set;
				set.script = script;
				set.lang = lang;
				set.gpos_features.clear();
				set.gsub_features = QStringList ( "aalt" );
				setList << set;
				qDebug() << "AALT"<<script<< lang;
			}
		}
	}

	QString spec;
	spec = QChar ( ccode );

	foreach ( OTFSet set, setList )
	{
		QList<RenderedGlyph> rendered ( otf->procstring ( spec, set ) );
		if ( rendered.at ( 0 ).glyph != glyphIndex )
		{
			if ( !ret.contains ( rendered.at ( 0 ).glyph ) )
				ret << rendered.at ( 0 ).glyph;
		}
		if ( !otf->altGlyphs.isEmpty() )
		{
			QList<int> l ( otf->altGlyphs );
			foreach ( int g, l )
			{
				if ( !ret.contains ( g ) && g != glyphIndex )
					ret << g;
			}
		}
	}

	delete otf;
	otf = 0;

	releaseFace();
	return ret;
}

QImage FontItem::glyphImage()
{
	QImage img ( m_face->glyph->bitmap.buffer,
	             m_face->glyph->bitmap.width,
	             m_face->glyph->bitmap.rows,
	             m_face->glyph->bitmap.pitch,
	             QImage::Format_Indexed8 );

	if ( m_face->glyph->bitmap.num_grays != 256 )
	{
		QVector<QRgb> palette;
		palette.clear();
		for ( int aa = 0; aa < m_face->glyph->bitmap.num_grays; ++aa )
		{
			palette << qRgba ( 0,0,0, aa );
		}
		img.setColorTable ( palette );
	}
	else
	{
		img.setColorTable ( gray256Palette );
	}

	return img;
}


QMap< int, QMap < QString , QString > > & FontItem::rawInfo()
{
	if ( moreInfo.isEmpty() )
	{
// 		qDebug()<< m_name <<"WARNING MoreInfo is empty";
		if ( m_isOpenType = true /*testFlag ( m_face->face_flags, FT_FACE_FLAG_SFNT, "1","0" ) == "1" */ )
		{
			moreInfo_sfnt();
		}
		if ( m_path.endsWith ( ".pfb",Qt::CaseInsensitive ) )
		{
			moreInfo_type1();
		}
	}
	return moreInfo;
}






int FontItem::shaperType() const
{
	return m_shaperType;
}


void FontItem::setShaperType ( int theValue )
{
	m_shaperType = theValue;
}
