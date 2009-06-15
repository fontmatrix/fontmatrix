//
// C++ Implementation: fmencdata
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmencdata.h"

#include <QLocale>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_IDS_H

FMEncData * FMEncData::instance = 0;
FMEncData::FMEncData()
{
	fillLangIdMap();
}

FMEncData * FMEncData::that()
{
	if(!instance)
	{
		instance = new FMEncData();
		Q_ASSERT(instance);
	}
	return instance;
}

void FMEncData::fillLangIdMap()
{
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

// 	// Now, what’s the locale code? so we’ll be able to get rid of unused data
// 	QString sysLang ( QLocale::languageToString ( QLocale::system ().language() ).toUpper() );
// 	QString sysCountry ( QLocale::countryToString ( QLocale::system ().country() ).toUpper() );
// 	int langIdMatch ( 0 );
// 	QMap<int,QString>::const_iterator lit;
// 	QList<int> locCodes;
// 	for ( lit = langIdMap.constBegin(); lit != langIdMap.constEnd() ;++lit )
// 	{
// 		if ( lit.value().startsWith ( sysLang ) )
// 			locCodes << lit.key();
// 	}
// 	langIdMatch = locCodes.value ( 0 );
// 	for ( int i ( 0 ); i < locCodes.count(); ++i )
// 	{
// 		if ( langIdMap[locCodes.at ( i ) ].contains ( sysCountry ) )
// 		{
// 			langIdMatch = locCodes.at ( i );
// 		}
// 	}

}

const QMap< int, QString > & FMEncData::LangIdMap()
{
	return that()->langIdMap;
}

