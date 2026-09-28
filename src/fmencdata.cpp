/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmencdata.h"

#include <QLocale>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_IDS_H

FMEncData *FMEncData::instance = nullptr;
FMEncData::FMEncData()
{
    fillLangIdMap();
    fillOs2URAnges();
}

FMEncData *FMEncData::that()
{
    if (!instance) {
        instance = new FMEncData();
        Q_ASSERT(instance);
    }
    return instance;
}

void FMEncData::fillLangIdMap()
{
    langIdMap[0x0000] = QLatin1String("DEFAULT");
    langIdMap[0x0001] = QLatin1String("ARABIC_GENERAL");
    langIdMap[0x0401] = QLatin1String("ARABIC_SAUDI_ARABIA");
    langIdMap[0x0801] = QLatin1String("ARABIC_IRAQ");
    langIdMap[0x0c01] = QLatin1String("ARABIC_EGYPT");
    langIdMap[0x1001] = QLatin1String("ARABIC_LIBYA");
    langIdMap[0x1401] = QLatin1String("ARABIC_ALGERIA");
    langIdMap[0x1801] = QLatin1String("ARABIC_MOROCCO");
    langIdMap[0x1c01] = QLatin1String("ARABIC_TUNISIA");
    langIdMap[0x2001] = QLatin1String("ARABIC_OMAN");
    langIdMap[0x2401] = QLatin1String("ARABIC_YEMEN");
    langIdMap[0x2801] = QLatin1String("ARABIC_SYRIA");
    langIdMap[0x2c01] = QLatin1String("ARABIC_JORDAN");
    langIdMap[0x3001] = QLatin1String("ARABIC_LEBANON");
    langIdMap[0x3401] = QLatin1String("ARABIC_KUWAIT");
    langIdMap[0x3801] = QLatin1String("ARABIC_UAE");
    langIdMap[0x3c01] = QLatin1String("ARABIC_BAHRAIN");
    langIdMap[0x4001] = QLatin1String("ARABIC_QATAR");
    langIdMap[0x0402] = QLatin1String("BULGARIAN_BULGARIA");
    langIdMap[0x0403] = QLatin1String("CATALAN_SPAIN");
    langIdMap[0x0004] = QLatin1String("CHINESE_GENERAL");
    langIdMap[0x0404] = QLatin1String("CHINESE_TAIWAN");
    langIdMap[0x0804] = QLatin1String("CHINESE_PRC");
    langIdMap[0x0c04] = QLatin1String("CHINESE_HONG_KONG");
    langIdMap[0x1004] = QLatin1String("CHINESE_SINGAPORE");
    langIdMap[0x1404] = QLatin1String("CHINESE_MACAU");
    langIdMap[TT_MS_LANGID_CHINESE_HONG_KONG] = QLatin1String("CHINESE_MACAU");
    langIdMap[0x7C04] = QLatin1String("CHINESE_TRADITIONAL");
    langIdMap[0x0405] = QLatin1String("CZECH_CZECH_REPUBLIC");
    langIdMap[0x0406] = QLatin1String("DANISH_DENMARK");
    langIdMap[0x0407] = QLatin1String("GERMAN_GERMANY");
    langIdMap[0x0807] = QLatin1String("GERMAN_SWITZERLAND");
    langIdMap[0x0c07] = QLatin1String("GERMAN_AUSTRIA");
    langIdMap[0x1007] = QLatin1String("GERMAN_LUXEMBOURG");
    langIdMap[0x1407] = QLatin1String("GERMAN_LIECHTENSTEI");
    langIdMap[0x0408] = QLatin1String("GREEK_GREECE");
    langIdMap[0x2008] = QLatin1String("GREEK_GREECE2");
    langIdMap[0x0009] = QLatin1String("ENGLISH_GENERAL");
    langIdMap[0x0409] = QLatin1String("ENGLISH_UNITED_STATES");
    langIdMap[0x0809] = QLatin1String("ENGLISH_UNITED_KINGDOM");
    langIdMap[0x0c09] = QLatin1String("ENGLISH_AUSTRALIA");
    langIdMap[0x1009] = QLatin1String("ENGLISH_CANADA");
    langIdMap[0x1409] = QLatin1String("ENGLISH_NEW_ZEALAND");
    langIdMap[0x1809] = QLatin1String("ENGLISH_IRELAND");
    langIdMap[0x1c09] = QLatin1String("ENGLISH_SOUTH_AFRICA");
    langIdMap[0x2009] = QLatin1String("ENGLISH_JAMAICA");
    langIdMap[0x2409] = QLatin1String("ENGLISH_CARIBBEAN");
    langIdMap[0x2809] = QLatin1String("ENGLISH_BELIZE");
    langIdMap[0x2c09] = QLatin1String("ENGLISH_TRINIDAD");
    langIdMap[0x3009] = QLatin1String("ENGLISH_ZIMBABWE");
    langIdMap[0x3409] = QLatin1String("ENGLISH_PHILIPPINES");
    langIdMap[0x3809] = QLatin1String("ENGLISH_INDONESIA");
    langIdMap[0x3c09] = QLatin1String("ENGLISH_HONG_KONG");
    langIdMap[0x4009] = QLatin1String("ENGLISH_INDIA");
    langIdMap[0x4409] = QLatin1String("ENGLISH_MALAYSIA");
    langIdMap[0x4809] = QLatin1String("ENGLISH_SINGAPORE");
    langIdMap[0x040a] = QLatin1String("SPANISH_SPAIN_TRADITIONAL_SORT");
    langIdMap[0x080a] = QLatin1String("SPANISH_MEXICO");
    langIdMap[0x0c0a] = QLatin1String("SPANISH_SPAIN_INTERNATIONAL_SORT");
    langIdMap[0x100a] = QLatin1String("SPANISH_GUATEMALA");
    langIdMap[0x140a] = QLatin1String("SPANISH_COSTA_RICA");
    langIdMap[0x180a] = QLatin1String("SPANISH_PANAMA");
    langIdMap[0x1c0a] = QLatin1String("SPANISH_DOMINICAN_REPUBLIC");
    langIdMap[0x200a] = QLatin1String("SPANISH_VENEZUELA");
    langIdMap[0x240a] = QLatin1String("SPANISH_COLOMBIA");
    langIdMap[0x280a] = QLatin1String("SPANISH_PERU");
    langIdMap[0x2c0a] = QLatin1String("SPANISH_ARGENTINA");
    langIdMap[0x300a] = QLatin1String("SPANISH_ECUADOR");
    langIdMap[0x340a] = QLatin1String("SPANISH_CHILE");
    langIdMap[0x380a] = QLatin1String("SPANISH_URUGUAY");
    langIdMap[0x3c0a] = QLatin1String("SPANISH_PARAGUAY");
    langIdMap[0x400a] = QLatin1String("SPANISH_BOLIVIA");
    langIdMap[0x440a] = QLatin1String("SPANISH_EL_SALVADOR");
    langIdMap[0x480a] = QLatin1String("SPANISH_HONDURAS");
    langIdMap[0x4c0a] = QLatin1String("SPANISH_NICARAGUA");
    langIdMap[0x500a] = QLatin1String("SPANISH_PUERTO_RICO");
    langIdMap[0x540a] = QLatin1String("SPANISH_UNITED_STATES");
    langIdMap[0xE40a] = QLatin1String("SPANISH_LATIN_AMERICA");
    langIdMap[0x040b] = QLatin1String("FINNISH_FINLAND");
    langIdMap[0x040c] = QLatin1String("FRENCH_FRANCE");
    langIdMap[0x080c] = QLatin1String("FRENCH_BELGIUM");
    langIdMap[0x0c0c] = QLatin1String("FRENCH_CANADA");
    langIdMap[0x100c] = QLatin1String("FRENCH_SWITZERLAND");
    langIdMap[0x140c] = QLatin1String("FRENCH_LUXEMBOURG");
    langIdMap[0x180c] = QLatin1String("FRENCH_MONACO");
    langIdMap[0x1c0c] = QLatin1String("FRENCH_WEST_INDIES");
    langIdMap[0x200c] = QLatin1String("FRENCH_REUNION");
    langIdMap[0x240c] = QLatin1String("FRENCH_CONGO");
    langIdMap[TT_MS_LANGID_FRENCH_CONGO] = QLatin1String("FRENCH_ZAIRE");
    langIdMap[0x280c] = QLatin1String("FRENCH_SENEGAL");
    langIdMap[0x2c0c] = QLatin1String("FRENCH_CAMEROON");
    langIdMap[0x300c] = QLatin1String("FRENCH_COTE_D_IVOIRE");
    langIdMap[0x340c] = QLatin1String("FRENCH_MALI");
    langIdMap[0x380c] = QLatin1String("FRENCH_MOROCCO");
    langIdMap[0x3c0c] = QLatin1String("FRENCH_HAITI");
    langIdMap[0xE40c] = QLatin1String("FRENCH_NORTH_AFRICA");
    langIdMap[0x040d] = QLatin1String("HEBREW_ISRAEL");
    langIdMap[0x040e] = QLatin1String("HUNGARIAN_HUNGARY");
    langIdMap[0x040f] = QLatin1String("ICELANDIC_ICELAND");
    langIdMap[0x0410] = QLatin1String("ITALIAN_ITALY");
    langIdMap[0x0810] = QLatin1String("ITALIAN_SWITZERLAND");
    langIdMap[0x0411] = QLatin1String("JAPANESE_JAPAN");
    langIdMap[0x0412] = QLatin1String("KOREAN_EXTENDED_WANSUNG_KOREA");
    langIdMap[0x0812] = QLatin1String("KOREAN_JOHAB_KOREA");
    langIdMap[0x0413] = QLatin1String("DUTCH_NETHERLANDS");
    langIdMap[0x0813] = QLatin1String("DUTCH_BELGIUM");
    langIdMap[0x0414] = QLatin1String("NORWEGIAN_NORWAY_BOKMAL");
    langIdMap[0x0814] = QLatin1String("NORWEGIAN_NORWAY_NYNORSK");
    langIdMap[0x0415] = QLatin1String("POLISH_POLAND");
    langIdMap[0x0416] = QLatin1String("PORTUGUESE_BRAZIL");
    langIdMap[0x0816] = QLatin1String("PORTUGUESE_PORTUGAL");
    langIdMap[0x0417] = QLatin1String("RHAETO_ROMANIC_SWITZERLAND");
    langIdMap[0x0418] = QLatin1String("ROMANIAN_ROMANIA");
    langIdMap[0x0818] = QLatin1String("MOLDAVIAN_MOLDAVIA");
    langIdMap[0x0419] = QLatin1String("RUSSIAN_RUSSIA");
    langIdMap[0x0819] = QLatin1String("RUSSIAN_MOLDAVIA");
    langIdMap[0x041a] = QLatin1String("CROATIAN_CROATIA");
    langIdMap[0x081a] = QLatin1String("SERBIAN_SERBIA_LATIN");
    langIdMap[0x0c1a] = QLatin1String("SERBIAN_SERBIA_CYRILLIC");
    langIdMap[0x101a] = QLatin1String("BOSNIAN_BOSNIA_HERZEGOVINA");
    langIdMap[0x101a] = QLatin1String("CROATIAN_BOSNIA_HERZEGOVINA");
    langIdMap[0x141a] = QLatin1String("BOSNIAN_BOSNIA_HERZEGOVINA");
    langIdMap[0x181a] = QLatin1String("SERBIAN_BOSNIA_HERZ_LATIN");
    langIdMap[0x181a] = QLatin1String("SERBIAN_BOSNIA_HERZ_CYRILLIC");
    langIdMap[0x041b] = QLatin1String("SLOVAK_SLOVAKIA");
    langIdMap[0x041c] = QLatin1String("ALBANIAN_ALBANIA");
    langIdMap[0x041d] = QLatin1String("SWEDISH_SWEDEN");
    langIdMap[0x081d] = QLatin1String("SWEDISH_FINLAND");
    langIdMap[0x041e] = QLatin1String("THAI_THAILAND");
    langIdMap[0x041f] = QLatin1String("TURKISH_TURKEY");
    langIdMap[0x0420] = QLatin1String("URDU_PAKISTAN");
    langIdMap[0x0820] = QLatin1String("URDU_INDIA");
    langIdMap[0x0421] = QLatin1String("INDONESIAN_INDONESIA");
    langIdMap[0x0422] = QLatin1String("UKRAINIAN_UKRAINE");
    langIdMap[0x0423] = QLatin1String("BELARUSIAN_BELARUS");
    langIdMap[0x0424] = QLatin1String("SLOVENE_SLOVENIA");
    langIdMap[0x0425] = QLatin1String("ESTONIAN_ESTONIA");
    langIdMap[0x0426] = QLatin1String("LATVIAN_LATVIA");
    langIdMap[0x0427] = QLatin1String("LITHUANIAN_LITHUANIA");
    langIdMap[0x0827] = QLatin1String("CLASSIC_LITHUANIAN_LITHUANIA");
    langIdMap[0x0428] = QLatin1String("TAJIK_TAJIKISTAN");
    langIdMap[0x0429] = QLatin1String("FARSI_IRAN");
    langIdMap[0x042a] = QLatin1String("VIETNAMESE_VIET_NAM");
    langIdMap[0x042b] = QLatin1String("ARMENIAN_ARMENIA");
    langIdMap[0x042c] = QLatin1String("AZERI_AZERBAIJAN_LATIN");
    langIdMap[0x082c] = QLatin1String("AZERI_AZERBAIJAN_CYRILLIC");
    langIdMap[0x042d] = QLatin1String("BASQUE_SPAIN");
    langIdMap[0x042e] = QLatin1String("SORBIAN_GERMANY");
    langIdMap[0x042f] = QLatin1String("MACEDONIAN_MACEDONIA");
    langIdMap[0x0430] = QLatin1String("SUTU_SOUTH_AFRICA");
    langIdMap[0x0431] = QLatin1String("TSONGA_SOUTH_AFRICA");
    langIdMap[0x0432] = QLatin1String("TSWANA_SOUTH_AFRICA");
    langIdMap[0x0433] = QLatin1String("VENDA_SOUTH_AFRICA");
    langIdMap[0x0434] = QLatin1String("XHOSA_SOUTH_AFRICA");
    langIdMap[0x0435] = QLatin1String("ZULU_SOUTH_AFRICA");
    langIdMap[0x0436] = QLatin1String("AFRIKAANS_SOUTH_AFRICA");
    langIdMap[0x0437] = QLatin1String("GEORGIAN_GEORGIA");
    langIdMap[0x0438] = QLatin1String("FAEROESE_FAEROE_ISLANDS");
    langIdMap[0x0439] = QLatin1String("HINDI_INDIA");
    langIdMap[0x043a] = QLatin1String("MALTESE_MALTA");
    langIdMap[0x043b] = QLatin1String("SAMI_NORTHERN_NORWAY");
    langIdMap[0x083b] = QLatin1String("SAMI_NORTHERN_SWEDEN");
    langIdMap[0x0C3b] = QLatin1String("SAMI_NORTHERN_FINLAND");
    langIdMap[0x103b] = QLatin1String("SAMI_LULE_NORWAY");
    langIdMap[0x143b] = QLatin1String("SAMI_LULE_SWEDEN");
    langIdMap[0x183b] = QLatin1String("SAMI_SOUTHERN_NORWAY");
    langIdMap[0x1C3b] = QLatin1String("SAMI_SOUTHERN_SWEDEN");
    langIdMap[0x203b] = QLatin1String("SAMI_SKOLT_FINLAND");
    langIdMap[0x243b] = QLatin1String("SAMI_INARI_FINLAND");
    langIdMap[0x043b] = QLatin1String("SAAMI_LAPONIA");
    langIdMap[0x043c] = QLatin1String("IRISH_GAELIC_IRELAND");
    langIdMap[0x083c] = QLatin1String("SCOTTISH_GAELIC_UNITED_KINGDOM");
    langIdMap[0x083c] = QLatin1String("SCOTTISH_GAELIC_UNITED_KINGDOM");
    langIdMap[0x043c] = QLatin1String("IRISH_GAELIC_IRELAND");
    langIdMap[0x043d] = QLatin1String("YIDDISH_GERMANY");
    langIdMap[0x043e] = QLatin1String("MALAY_MALAYSIA");
    langIdMap[0x083e] = QLatin1String("MALAY_BRUNEI_DARUSSALAM");
    langIdMap[0x043f] = QLatin1String("KAZAK_KAZAKSTAN");
    langIdMap[0x0440] = QLatin1String("KIRGHIZ_KIRGHIZSTAN");
    langIdMap[0x0441] = QLatin1String("SWAHILI_KENYA");
    langIdMap[0x0442] = QLatin1String("TURKMEN_TURKMENISTAN");
    langIdMap[0x0443] = QLatin1String("UZBEK_UZBEKISTAN_LATIN");
    langIdMap[0x0843] = QLatin1String("UZBEK_UZBEKISTAN_CYRILLIC");
    langIdMap[0x0444] = QLatin1String("TATAR_TATARSTAN");
    langIdMap[0x0445] = QLatin1String("BENGALI_INDIA");
    langIdMap[0x0845] = QLatin1String("BENGALI_BANGLADESH");
    langIdMap[0x0446] = QLatin1String("PUNJABI_INDIA");
    langIdMap[0x0846] = QLatin1String("PUNJABI_ARABIC_PAKISTAN");
    langIdMap[0x0447] = QLatin1String("GUJARATI_INDIA");
    langIdMap[0x0448] = QLatin1String("ORIYA_INDIA");
    langIdMap[0x0449] = QLatin1String("TAMIL_INDIA");
    langIdMap[0x044a] = QLatin1String("TELUGU_INDIA");
    langIdMap[0x044b] = QLatin1String("KANNADA_INDIA");
    langIdMap[0x044c] = QLatin1String("MALAYALAM_INDIA");
    langIdMap[0x044d] = QLatin1String("ASSAMESE_INDIA");
    langIdMap[0x044e] = QLatin1String("MARATHI_INDIA");
    langIdMap[0x044f] = QLatin1String("SANSKRIT_INDIA");
    langIdMap[0x0450] = QLatin1String("MONGOLIAN_MONGOLIA/*Cyrillic*/");
    langIdMap[0x0850] = QLatin1String("MONGOLIAN_MONGOLIA_MONGOLIAN");
    langIdMap[0x0451] = QLatin1String("TIBETAN_CHINA");
    /*TT_MS_LANGID_TIBETAN_BHUTANiscorrect,BTW.*/
    langIdMap[0x0851] = QLatin1String("DZONGHKA_BHUTAN");
    langIdMap[0x0451] = QLatin1String("TIBETAN_BHUTAN");
    langIdMap[TT_MS_LANGID_DZONGHKA_BHUTAN] = QLatin1String("TIBETAN_BHUTAN");
    langIdMap[0x0452] = QLatin1String("WELSH_WALES");
    langIdMap[0x0453] = QLatin1String("KHMER_CAMBODIA");
    langIdMap[0x0454] = QLatin1String("LAO_LAOS");
    langIdMap[0x0455] = QLatin1String("BURMESE_MYANMAR");
    langIdMap[0x0456] = QLatin1String("GALICIAN_SPAIN");
    langIdMap[0x0457] = QLatin1String("KONKANI_INDIA");
    langIdMap[0x0458] = QLatin1String("MANIPURI_INDIA/*Bengali*/");
    langIdMap[0x0459] = QLatin1String("SINDHI_INDIA/*Arabic*/");
    langIdMap[0x0859] = QLatin1String("SINDHI_PAKISTAN");
    langIdMap[0x045a] = QLatin1String("SYRIAC_SYRIA");
    langIdMap[0x045b] = QLatin1String("SINHALESE_SRI_LANKA");
    langIdMap[0x045c] = QLatin1String("CHEROKEE_UNITED_STATES");
    langIdMap[0x045d] = QLatin1String("INUKTITUT_CANADA");
    langIdMap[0x045e] = QLatin1String("AMHARIC_ETHIOPIA");
    langIdMap[0x045f] = QLatin1String("TAMAZIGHT_MOROCCO/*Arabic*/");
    langIdMap[0x085f] = QLatin1String("TAMAZIGHT_MOROCCO_LATIN");
    langIdMap[0x0460] = QLatin1String("KASHMIRI_PAKISTAN/*Arabic*/");
    langIdMap[0x0860] = QLatin1String("KASHMIRI_SASIA");
    langIdMap[TT_MS_LANGID_KASHMIRI_SASIA] = QLatin1String("KASHMIRI_INDIA");
    langIdMap[0x0461] = QLatin1String("NEPALI_NEPAL");
    langIdMap[0x0861] = QLatin1String("NEPALI_INDIA");
    langIdMap[0x0462] = QLatin1String("FRISIAN_NETHERLANDS");
    langIdMap[0x0463] = QLatin1String("PASHTO_AFGHANISTAN");
    langIdMap[0x0464] = QLatin1String("FILIPINO_PHILIPPINES");
    langIdMap[0x0465] = QLatin1String("DHIVEHI_MALDIVES");
    langIdMap[TT_MS_LANGID_DHIVEHI_MALDIVES] = QLatin1String("DIVEHI_MALDIVES");
    langIdMap[0x0466] = QLatin1String("EDO_NIGERIA");
    langIdMap[0x0467] = QLatin1String("FULFULDE_NIGERIA");
    langIdMap[0x0468] = QLatin1String("HAUSA_NIGERIA");
    langIdMap[0x0469] = QLatin1String("IBIBIO_NIGERIA");
    langIdMap[0x046a] = QLatin1String("YORUBA_NIGERIA");
    langIdMap[0x046b] = QLatin1String("QUECHUA_BOLIVIA");
    langIdMap[0x086b] = QLatin1String("QUECHUA_ECUADOR");
    langIdMap[0x0c6b] = QLatin1String("QUECHUA_PERU");
    langIdMap[0x046c] = QLatin1String("SEPEDI_SOUTH_AFRICA");
    langIdMap[0x0470] = QLatin1String("IGBO_NIGERIA");
    langIdMap[0x0471] = QLatin1String("KANURI_NIGERIA");
    langIdMap[0x0472] = QLatin1String("OROMO_ETHIOPIA");
    langIdMap[0x0473] = QLatin1String("TIGRIGNA_ETHIOPIA");
    langIdMap[0x0873] = QLatin1String("TIGRIGNA_ERYTHREA");
    langIdMap[TT_MS_LANGID_TIGRIGNA_ERYTHREA] = QLatin1String("TIGRIGNA_ERYTREA");
    langIdMap[0x0474] = QLatin1String("GUARANI_PARAGUAY");
    langIdMap[0x0475] = QLatin1String("HAWAIIAN_UNITED_STATES");
    langIdMap[0x0476] = QLatin1String("LATIN");
    langIdMap[0x0477] = QLatin1String("SOMALI_SOMALIA");
    langIdMap[0x0478] = QLatin1String("YI_CHINA");
    langIdMap[0x0479] = QLatin1String("PAPIAMENTU_NETHERLANDS_ANTILLES");
    langIdMap[0x0480] = QLatin1String("UIGHUR_CHINA");
    langIdMap[0x0481] = QLatin1String("MAORI_NEW_ZEALAND");
    langIdMap[0x04ff] = QLatin1String("HUMAN_INTERFACE_DEVICE");

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

void FMEncData::fillOs2URAnges()
{
    os2URangeMap[0] = qMakePair<int, int>(0x0000, 0x007F);
    os2URangeMap[1] = qMakePair<int, int>(0x0080, 0x00FF);
    os2URangeMap[2] = qMakePair<int, int>(0x0100, 0x017F);
    os2URangeMap[3] = qMakePair<int, int>(0x0180, 0x024F);
    os2URangeMap[4] = qMakePair<int, int>(0x0250, 0x02AF);
    os2URangeMap[5] = qMakePair<int, int>(0x02B0, 0x02FF);
    os2URangeMap[6] = qMakePair<int, int>(0x0300, 0x036F);
    os2URangeMap[7] = qMakePair<int, int>(0x0370, 0x03FF);
    os2URangeMap[8] = qMakePair<int, int>(0x2C80, 0x2CFF);
    os2URangeMap[9] = qMakePair<int, int>(0x0400, 0x04FF);
    os2URangeMap[10] = qMakePair<int, int>(0x0530, 0x058F);
    os2URangeMap[11] = qMakePair<int, int>(0x0590, 0x05FF);
    os2URangeMap[12] = qMakePair<int, int>(0xA500, 0xA63F);
    os2URangeMap[13] = qMakePair<int, int>(0x0600, 0x06FF);
    os2URangeMap[14] = qMakePair<int, int>(0x07C0, 0x07FF);
    os2URangeMap[15] = qMakePair<int, int>(0x0900, 0x097F);
    os2URangeMap[16] = qMakePair<int, int>(0x0980, 0x09FF);
    os2URangeMap[17] = qMakePair<int, int>(0x0A00, 0x0A7F);
    os2URangeMap[18] = qMakePair<int, int>(0x0A80, 0x0AFF);
    os2URangeMap[19] = qMakePair<int, int>(0x0B00, 0x0B7F);
    os2URangeMap[20] = qMakePair<int, int>(0x0B80, 0x0BFF);
    os2URangeMap[21] = qMakePair<int, int>(0x0C00, 0x0C7F);
    os2URangeMap[22] = qMakePair<int, int>(0x0C80, 0x0CFF);
    os2URangeMap[23] = qMakePair<int, int>(0x0D00, 0x0D7F);
    os2URangeMap[24] = qMakePair<int, int>(0x0E00, 0x0E7F);
    os2URangeMap[25] = qMakePair<int, int>(0x0E80, 0x0EFF);
    os2URangeMap[26] = qMakePair<int, int>(0x10A0, 0x10FF);
    os2URangeMap[27] = qMakePair<int, int>(0x1B00, 0x1B7F);
    os2URangeMap[28] = qMakePair<int, int>(0x1100, 0x11FF);
    os2URangeMap[29] = qMakePair<int, int>(0x1E00, 0x1EFF);
    os2URangeMap[30] = qMakePair<int, int>(0x1F00, 0x1FFF);
    os2URangeMap[31] = qMakePair<int, int>(0x2000, 0x206F);
    os2URangeMap[32] = qMakePair<int, int>(0x2070, 0x209F);
    os2URangeMap[33] = qMakePair<int, int>(0x20A0, 0x20CF);
    os2URangeMap[34] = qMakePair<int, int>(0x20D0, 0x20FF);
    os2URangeMap[35] = qMakePair<int, int>(0x2100, 0x214F);
    os2URangeMap[36] = qMakePair<int, int>(0x2150, 0x218F);
    os2URangeMap[37] = qMakePair<int, int>(0x2190, 0x21FF);
    os2URangeMap[38] = qMakePair<int, int>(0x2200, 0x22FF);
    os2URangeMap[39] = qMakePair<int, int>(0x2300, 0x23FF);
    os2URangeMap[40] = qMakePair<int, int>(0x2400, 0x243F);
    os2URangeMap[41] = qMakePair<int, int>(0x2440, 0x245F);
    os2URangeMap[42] = qMakePair<int, int>(0x2460, 0x24FF);
    os2URangeMap[43] = qMakePair<int, int>(0x2500, 0x257F);
    os2URangeMap[44] = qMakePair<int, int>(0x2580, 0x259F);
    os2URangeMap[45] = qMakePair<int, int>(0x25A0, 0x25FF);
    os2URangeMap[46] = qMakePair<int, int>(0x2600, 0x26FF);
    os2URangeMap[47] = qMakePair<int, int>(0x2700, 0x27BF);
    os2URangeMap[48] = qMakePair<int, int>(0x3000, 0x303F);
    os2URangeMap[49] = qMakePair<int, int>(0x3040, 0x309F);
    os2URangeMap[50] = qMakePair<int, int>(0x30A0, 0x30FF);
    os2URangeMap[51] = qMakePair<int, int>(0x3100, 0x312F);
    os2URangeMap[52] = qMakePair<int, int>(0x3130, 0x318F);
    os2URangeMap[53] = qMakePair<int, int>(0xA840, 0xA87F);
    os2URangeMap[54] = qMakePair<int, int>(0x3200, 0x32FF);
    os2URangeMap[55] = qMakePair<int, int>(0x3300, 0x33FF);
    os2URangeMap[56] = qMakePair<int, int>(0xAC00, 0xD7AF);
    os2URangeMap[58] = qMakePair<int, int>(0x10900, 0x1091F);
    os2URangeMap[59] = qMakePair<int, int>(0x4E00, 0x9FFF);
    os2URangeMap[60] = qMakePair<int, int>(0xE000, 0xF8FF);
    os2URangeMap[61] = qMakePair<int, int>(0x31C0, 0x31EF);
    os2URangeMap[62] = qMakePair<int, int>(0xFB00, 0xFB4F);
    os2URangeMap[63] = qMakePair<int, int>(0xFB50, 0xFDFF);
    os2URangeMap[64] = qMakePair<int, int>(0xFE20, 0xFE2F);
    os2URangeMap[65] = qMakePair<int, int>(0xFE10, 0xFE1F);
    os2URangeMap[66] = qMakePair<int, int>(0xFE50, 0xFE6F);
    os2URangeMap[67] = qMakePair<int, int>(0xFE70, 0xFEFF);
    os2URangeMap[68] = qMakePair<int, int>(0xFF00, 0xFFEF);
    os2URangeMap[69] = qMakePair<int, int>(0xFFF0, 0xFFFF);
    os2URangeMap[70] = qMakePair<int, int>(0x0F00, 0x0FFF);
    os2URangeMap[71] = qMakePair<int, int>(0x0700, 0x074F);
    os2URangeMap[72] = qMakePair<int, int>(0x0780, 0x07BF);
    os2URangeMap[73] = qMakePair<int, int>(0x0D80, 0x0DFF);
    os2URangeMap[74] = qMakePair<int, int>(0x1000, 0x109F);
    os2URangeMap[75] = qMakePair<int, int>(0x1200, 0x137F);
    os2URangeMap[76] = qMakePair<int, int>(0x13A0, 0x13FF);
    os2URangeMap[77] = qMakePair<int, int>(0x1400, 0x167F);
    os2URangeMap[78] = qMakePair<int, int>(0x1680, 0x169F);
    os2URangeMap[79] = qMakePair<int, int>(0x16A0, 0x16FF);
    os2URangeMap[80] = qMakePair<int, int>(0x1780, 0x17FF);
    os2URangeMap[81] = qMakePair<int, int>(0x1800, 0x18AF);
    os2URangeMap[82] = qMakePair<int, int>(0x2800, 0x28FF);
    os2URangeMap[83] = qMakePair<int, int>(0xA000, 0xA48F);
    os2URangeMap[84] = qMakePair<int, int>(0x1700, 0x171F);
    os2URangeMap[85] = qMakePair<int, int>(0x10300, 0x1032F);
    os2URangeMap[86] = qMakePair<int, int>(0x10330, 0x1034F);
    os2URangeMap[87] = qMakePair<int, int>(0x10400, 0x1044F);
    os2URangeMap[88] = qMakePair<int, int>(0x1D000, 0x1D0FF);
    os2URangeMap[89] = qMakePair<int, int>(0x1D400, 0x1D7FF);
    os2URangeMap[90] = qMakePair<int, int>(0xFF000, 0xFFFFD);
    os2URangeMap[91] = qMakePair<int, int>(0xFE00, 0xFE0F);
    os2URangeMap[92] = qMakePair<int, int>(0xE0000, 0xE007F);
    os2URangeMap[93] = qMakePair<int, int>(0x1900, 0x194F);
    os2URangeMap[94] = qMakePair<int, int>(0x1950, 0x197F);
    os2URangeMap[95] = qMakePair<int, int>(0x1980, 0x19DF);
    os2URangeMap[96] = qMakePair<int, int>(0x1A00, 0x1A1F);
    os2URangeMap[97] = qMakePair<int, int>(0x2C00, 0x2C5F);
    os2URangeMap[98] = qMakePair<int, int>(0x2D30, 0x2D7F);
    os2URangeMap[99] = qMakePair<int, int>(0x4DC0, 0x4DFF);
    os2URangeMap[100] = qMakePair<int, int>(0xA800, 0xA82F);
    os2URangeMap[101] = qMakePair<int, int>(0x10000, 0x1007F);
    os2URangeMap[102] = qMakePair<int, int>(0x10140, 0x1018F);
    os2URangeMap[103] = qMakePair<int, int>(0x10380, 0x1039F);
    os2URangeMap[104] = qMakePair<int, int>(0x103A0, 0x103DF);
    os2URangeMap[105] = qMakePair<int, int>(0x10450, 0x1047F);
    os2URangeMap[106] = qMakePair<int, int>(0x10480, 0x104AF);
    os2URangeMap[107] = qMakePair<int, int>(0x10800, 0x1083F);
    os2URangeMap[108] = qMakePair<int, int>(0x10A00, 0x10A5F);
    os2URangeMap[109] = qMakePair<int, int>(0x1D300, 0x1D35F);
    os2URangeMap[110] = qMakePair<int, int>(0x12000, 0x123FF);
    os2URangeMap[111] = qMakePair<int, int>(0x1D360, 0x1D37F);
    os2URangeMap[112] = qMakePair<int, int>(0x1B80, 0x1BBF);
    os2URangeMap[113] = qMakePair<int, int>(0x1C00, 0x1C4F);
    os2URangeMap[114] = qMakePair<int, int>(0x1C50, 0x1C7F);
    os2URangeMap[115] = qMakePair<int, int>(0xA880, 0xA8DF);
    os2URangeMap[116] = qMakePair<int, int>(0xA900, 0xA92F);
    os2URangeMap[117] = qMakePair<int, int>(0xA930, 0xA95F);
    os2URangeMap[118] = qMakePair<int, int>(0xAA00, 0xAA5F);
    os2URangeMap[119] = qMakePair<int, int>(0x10190, 0x101CF);
    os2URangeMap[120] = qMakePair<int, int>(0x101D0, 0x101FF);
    os2URangeMap[121] = qMakePair<int, int>(0x102A0, 0x102DF);
    os2URangeMap[122] = qMakePair<int, int>(0x1F030, 0x1F09F);
}

const QMap<int, QString> &FMEncData::LangIdMap()
{
    return that()->langIdMap;
}

const QMap<int, QPair<int, int>> &FMEncData::Os2URanges()
{
    return that()->os2URangeMap;
}
