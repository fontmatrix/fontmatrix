/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OTTMEAN_H
#define OTTMEAN_H

#include <KLocalizedString>
#include <QMap>

QString OTTagMeans(QString tag)
{
    static QMap<QString, QString> OTTagsMeaning;
    if (OTTagsMeaning.isEmpty()) {
        OTTagsMeaning[QStringLiteral("aalt")] = i18nc("@item:intable OpenType feature", "Access All Alternates");
        OTTagsMeaning[QStringLiteral("abvf")] = i18nc("@item:intable OpenType feature", "Above-Base Forms");
        OTTagsMeaning[QStringLiteral("abvm")] = i18nc("@item:intable OpenType feature", "Above-Base Mark Positioning");
        OTTagsMeaning[QStringLiteral("abvs")] = i18nc("@item:intable OpenType feature", "Above-Base Substitutions");
        OTTagsMeaning[QStringLiteral("afrc")] = i18nc("@item:intable OpenType feature", "Alternative Fractions");
        OTTagsMeaning[QStringLiteral("akhn")] = i18nc("@item:intable OpenType feature", "Akhands");
        OTTagsMeaning[QStringLiteral("blwf")] = i18nc("@item:intable OpenType feature", "Below-Base Forms");
        OTTagsMeaning[QStringLiteral("blwm")] = i18nc("@item:intable OpenType feature", "Below-Base Mark Positioning");
        OTTagsMeaning[QStringLiteral("blws")] = i18nc("@item:intable OpenType feature", "Below-Base Substitutions");
        OTTagsMeaning[QStringLiteral("c2pc")] = i18nc("@item:intable OpenType feature", "Petite Capitals From Capitals");
        OTTagsMeaning[QStringLiteral("c2sc")] = i18nc("@item:intable OpenType feature", "Small Capitals From Capitals");
        OTTagsMeaning[QStringLiteral("calt")] = i18nc("@item:intable OpenType feature", "Contextual Alternates");
        OTTagsMeaning[QStringLiteral("case")] = i18nc("@item:intable OpenType feature", "Case-Sensitive Forms");
        OTTagsMeaning[QStringLiteral("ccmp")] = i18nc("@item:intable OpenType feature", "Glyph Composition/Decomposition");
        OTTagsMeaning[QStringLiteral("clig")] = i18nc("@item:intable OpenType feature", "Contextual Ligatures");
        OTTagsMeaning[QStringLiteral("cjct")] = i18nc("@item:intable OpenType feature", "Conjunct Forms");
        OTTagsMeaning[QStringLiteral("cpsp")] = i18nc("@item:intable OpenType feature", "Capital Spacing");
        OTTagsMeaning[QStringLiteral("cswh")] = i18nc("@item:intable OpenType feature", "Contextual Swash");
        OTTagsMeaning[QStringLiteral("curs")] = i18nc("@item:intable OpenType feature", "Cursive Positioning");
        OTTagsMeaning[QStringLiteral("dflt")] = i18nc("@item:intable OpenType feature", "Default Processing");
        OTTagsMeaning[QStringLiteral("dist")] = i18nc("@item:intable OpenType feature", "Distances");
        OTTagsMeaning[QStringLiteral("dlig")] = i18nc("@item:intable OpenType feature", "Discretionary Ligatures");
        OTTagsMeaning[QStringLiteral("dnom")] = i18nc("@item:intable OpenType feature", "Denominators");
        OTTagsMeaning[QStringLiteral("expt")] = i18nc("@item:intable OpenType feature", "Expert Forms");
        OTTagsMeaning[QStringLiteral("falt")] = i18nc("@item:intable OpenType feature", "Final glyph Alternates");
        OTTagsMeaning[QStringLiteral("fin2")] = i18nc("@item:intable OpenType feature", "Terminal Forms #2");
        OTTagsMeaning[QStringLiteral("fin3")] = i18nc("@item:intable OpenType feature", "Terminal Forms #3");
        OTTagsMeaning[QStringLiteral("fina")] = i18nc("@item:intable OpenType feature", "Terminal Forms");
        OTTagsMeaning[QStringLiteral("frac")] = i18nc("@item:intable OpenType feature", "Fractions");
        OTTagsMeaning[QStringLiteral("fwid")] = i18nc("@item:intable OpenType feature", "Full Width");
        OTTagsMeaning[QStringLiteral("half")] = i18nc("@item:intable OpenType feature", "Half Forms");
        OTTagsMeaning[QStringLiteral("haln")] = i18nc("@item:intable OpenType feature", "Halant Forms");
        OTTagsMeaning[QStringLiteral("halt")] = i18nc("@item:intable OpenType feature", "Alternate Half Width");
        OTTagsMeaning[QStringLiteral("hist")] = i18nc("@item:intable OpenType feature", "Historical Forms");
        OTTagsMeaning[QStringLiteral("hkna")] = i18nc("@item:intable OpenType feature", "Horizontal Kana Alternates");
        OTTagsMeaning[QStringLiteral("hlig")] = i18nc("@item:intable OpenType feature", "Historical Ligatures");
        OTTagsMeaning[QStringLiteral("hngl")] = i18nc("@item:intable OpenType feature", "Hangul");
        OTTagsMeaning[QStringLiteral("hojo")] = i18nc("@item:intable OpenType feature", "Hojo Kanji Forms (JIS x 212-1990 Kanji Forms)");
        OTTagsMeaning[QStringLiteral("hwid")] = i18nc("@item:intable OpenType feature", "Half Width");
        OTTagsMeaning[QStringLiteral("init")] = i18nc("@item:intable OpenType feature", "Initial Forms");
        OTTagsMeaning[QStringLiteral("isol")] = i18nc("@item:intable OpenType feature", "Isolated Forms");
        OTTagsMeaning[QStringLiteral("ital")] = i18nc("@item:intable OpenType feature", "Italics");
        OTTagsMeaning[QStringLiteral("jalt")] = i18nc("@item:intable OpenType feature", "Justification Alternatives");
        OTTagsMeaning[QStringLiteral("jp78")] = i18nc("@item:intable OpenType feature", "JIS78 Forms");
        OTTagsMeaning[QStringLiteral("jp83")] = i18nc("@item:intable OpenType feature", "JIS83 Forms");
        OTTagsMeaning[QStringLiteral("jp90")] = i18nc("@item:intable OpenType feature", "JIS90 Forms");
        OTTagsMeaning[QStringLiteral("jp04")] = i18nc("@item:intable OpenType feature", "JIS2004 Forms");
        OTTagsMeaning[QStringLiteral("kern")] = i18nc("@item:intable OpenType feature", "Kerning");
        OTTagsMeaning[QStringLiteral("lfbd")] = i18nc("@item:intable OpenType feature", "Left Bounds");
        OTTagsMeaning[QStringLiteral("liga")] = i18nc("@item:intable OpenType feature", "Standard Ligatures");
        OTTagsMeaning[QStringLiteral("ljmo")] = i18nc("@item:intable OpenType feature", "Leading Jamo Forms");
        OTTagsMeaning[QStringLiteral("lnum")] = i18nc("@item:intable OpenType feature", "Lining Figures");
        OTTagsMeaning[QStringLiteral("locl")] = i18nc("@item:intable OpenType feature", "Localized Forms");
        OTTagsMeaning[QStringLiteral("mark")] = i18nc("@item:intable OpenType feature", "Mark Positioning");
        OTTagsMeaning[QStringLiteral("med2")] = i18nc("@item:intable OpenType feature", "Medial Forms #2");
        OTTagsMeaning[QStringLiteral("medi")] = i18nc("@item:intable OpenType feature", "Medial Forms");
        OTTagsMeaning[QStringLiteral("mgrk")] = i18nc("@item:intable OpenType feature", "Mathematical Greek");
        OTTagsMeaning[QStringLiteral("mkmk")] = i18nc("@item:intable OpenType feature", "Mark to Mark Positioning");
        OTTagsMeaning[QStringLiteral("mset")] = i18nc("@item:intable OpenType feature", "Mark Positioning via Substitution");
        OTTagsMeaning[QStringLiteral("nalt")] = i18nc("@item:intable OpenType feature", "Alternate Annotation Forms");
        OTTagsMeaning[QStringLiteral("nlck")] = i18nc("@item:intable OpenType feature", "NLC Kanji Forms");
        OTTagsMeaning[QStringLiteral("nukt")] = i18nc("@item:intable OpenType feature", "Nukta Forms");
        OTTagsMeaning[QStringLiteral("numr")] = i18nc("@item:intable OpenType feature", "Numerators");
        OTTagsMeaning[QStringLiteral("onum")] = i18nc("@item:intable OpenType feature", "Old Style Figures");
        OTTagsMeaning[QStringLiteral("opbd")] = i18nc("@item:intable OpenType feature", "Optical Bounds");
        OTTagsMeaning[QStringLiteral("ordn")] = i18nc("@item:intable OpenType feature", "Ordinals");
        OTTagsMeaning[QStringLiteral("ornm")] = i18nc("@item:intable OpenType feature", "Ornaments");
        OTTagsMeaning[QStringLiteral("palt")] = i18nc("@item:intable OpenType feature", "Proportional Alternate Width");
        OTTagsMeaning[QStringLiteral("pcap")] = i18nc("@item:intable OpenType feature", "Petite Capitals");
        OTTagsMeaning[QStringLiteral("pnum")] = i18nc("@item:intable OpenType feature", "Proportional Figures");
        OTTagsMeaning[QStringLiteral("pref")] = i18nc("@item:intable OpenType feature", "Pre-base Forms");
        OTTagsMeaning[QStringLiteral("pres")] = i18nc("@item:intable OpenType feature", "Pre-base Substitutions");
        OTTagsMeaning[QStringLiteral("pstf")] = i18nc("@item:intable OpenType feature", "Post-base Forms");
        OTTagsMeaning[QStringLiteral("psts")] = i18nc("@item:intable OpenType feature", "Post-base Substitutions");
        OTTagsMeaning[QStringLiteral("pwid")] = i18nc("@item:intable OpenType feature", "Proportional Widths");
        OTTagsMeaning[QStringLiteral("qwid")] = i18nc("@item:intable OpenType feature", "Quarter Widths");
        OTTagsMeaning[QStringLiteral("rand")] = i18nc("@item:intable OpenType feature", "Randomize");
        OTTagsMeaning[QStringLiteral("rkrf")] = i18nc("@item:intable OpenType feature", "Rakar Forms");
        OTTagsMeaning[QStringLiteral("rlig")] = i18nc("@item:intable OpenType feature", "Required Ligatures");
        OTTagsMeaning[QStringLiteral("rphf")] = i18nc("@item:intable OpenType feature", "Reph Form");
        OTTagsMeaning[QStringLiteral("rtbd")] = i18nc("@item:intable OpenType feature", "Right Bounds");
        OTTagsMeaning[QStringLiteral("rtla")] = i18nc("@item:intable OpenType feature", "Right-To-Left Alternates");
        OTTagsMeaning[QStringLiteral("ruby")] = i18nc("@item:intable OpenType feature", "Ruby Notation Forms");
        OTTagsMeaning[QStringLiteral("salt")] = i18nc("@item:intable OpenType feature", "Stylistic Alternates");
        OTTagsMeaning[QStringLiteral("sinf")] = i18nc("@item:intable OpenType feature", "Scientific Inferiors");
        OTTagsMeaning[QStringLiteral("size")] = i18nc("@item:intable OpenType feature", "Optical Size");
        OTTagsMeaning[QStringLiteral("smcp")] = i18nc("@item:intable OpenType feature", "Small Capitals");
        OTTagsMeaning[QStringLiteral("smpl")] = i18nc("@item:intable OpenType feature", "Simplified Forms");
        OTTagsMeaning[QStringLiteral("ss01")] = i18nc("@item:intable OpenType feature", "Stylistic Set 1");
        OTTagsMeaning[QStringLiteral("ss02")] = i18nc("@item:intable OpenType feature", "Stylistic Set 2");
        OTTagsMeaning[QStringLiteral("ss03")] = i18nc("@item:intable OpenType feature", "Stylistic Set 3");
        OTTagsMeaning[QStringLiteral("ss04")] = i18nc("@item:intable OpenType feature", "Stylistic Set 4");
        OTTagsMeaning[QStringLiteral("ss05")] = i18nc("@item:intable OpenType feature", "Stylistic Set 5");
        OTTagsMeaning[QStringLiteral("ss06")] = i18nc("@item:intable OpenType feature", "Stylistic Set 6");
        OTTagsMeaning[QStringLiteral("ss07")] = i18nc("@item:intable OpenType feature", "Stylistic Set 7");
        OTTagsMeaning[QStringLiteral("ss08")] = i18nc("@item:intable OpenType feature", "Stylistic Set 8");
        OTTagsMeaning[QStringLiteral("ss09")] = i18nc("@item:intable OpenType feature", "Stylistic Set 9");
        OTTagsMeaning[QStringLiteral("ss10")] = i18nc("@item:intable OpenType feature", "Stylistic Set 10");
        OTTagsMeaning[QStringLiteral("ss11")] = i18nc("@item:intable OpenType feature", "Stylistic Set 11");
        OTTagsMeaning[QStringLiteral("ss12")] = i18nc("@item:intable OpenType feature", "Stylistic Set 12");
        OTTagsMeaning[QStringLiteral("ss13")] = i18nc("@item:intable OpenType feature", "Stylistic Set 13");
        OTTagsMeaning[QStringLiteral("ss14")] = i18nc("@item:intable OpenType feature", "Stylistic Set 14");
        OTTagsMeaning[QStringLiteral("ss15")] = i18nc("@item:intable OpenType feature", "Stylistic Set 15");
        OTTagsMeaning[QStringLiteral("ss16")] = i18nc("@item:intable OpenType feature", "Stylistic Set 16");
        OTTagsMeaning[QStringLiteral("ss17")] = i18nc("@item:intable OpenType feature", "Stylistic Set 17");
        OTTagsMeaning[QStringLiteral("ss18")] = i18nc("@item:intable OpenType feature", "Stylistic Set 18");
        OTTagsMeaning[QStringLiteral("ss19")] = i18nc("@item:intable OpenType feature", "Stylistic Set 19");
        OTTagsMeaning[QStringLiteral("ss20")] = i18nc("@item:intable OpenType feature", "Stylistic Set 20");
        OTTagsMeaning[QStringLiteral("subs")] = i18nc("@item:intable OpenType feature", "Subscript");
        OTTagsMeaning[QStringLiteral("sups")] = i18nc("@item:intable OpenType feature", "Superscript");
        OTTagsMeaning[QStringLiteral("swsh")] = i18nc("@item:intable OpenType feature", "Swash");
        OTTagsMeaning[QStringLiteral("titl")] = i18nc("@item:intable OpenType feature", "Titling");
        OTTagsMeaning[QStringLiteral("tjmo")] = i18nc("@item:intable OpenType feature", "Trailing Jamo Forms");
        OTTagsMeaning[QStringLiteral("tnam")] = i18nc("@item:intable OpenType feature", "Traditional Name Forms");
        OTTagsMeaning[QStringLiteral("tnum")] = i18nc("@item:intable OpenType feature", "Tabular Figures");
        OTTagsMeaning[QStringLiteral("trad")] = i18nc("@item:intable OpenType feature", "Traditional Forms");
        OTTagsMeaning[QStringLiteral("twid")] = i18nc("@item:intable OpenType feature", "Third Widths");
        OTTagsMeaning[QStringLiteral("unic")] = i18nc("@item:intable OpenType feature", "Unicase");
        OTTagsMeaning[QStringLiteral("valt")] = i18nc("@item:intable OpenType feature", "Alternate Vertical Metrics");
        OTTagsMeaning[QStringLiteral("vatu")] = i18nc("@item:intable OpenType feature", "Vattu Variants");
        OTTagsMeaning[QStringLiteral("vert")] = i18nc("@item:intable OpenType feature", "Vertical Writing");
        OTTagsMeaning[QStringLiteral("vhal")] = i18nc("@item:intable OpenType feature", "Alternate Vertical Half Metrics");
        OTTagsMeaning[QStringLiteral("vjmo")] = i18nc("@item:intable OpenType feature", "Vowel Jamo Forms");
        OTTagsMeaning[QStringLiteral("vkna")] = i18nc("@item:intable OpenType feature", "Vertical Kana Alternates");
        OTTagsMeaning[QStringLiteral("vkrn")] = i18nc("@item:intable OpenType feature", "Vertical Kerning");
        OTTagsMeaning[QStringLiteral("vpal")] = i18nc("@item:intable OpenType feature", "Proportional Alternate Vertical Metrics");
        OTTagsMeaning[QStringLiteral("vrt2")] = i18nc("@item:intable OpenType feature", "Vertical Rotation");
        OTTagsMeaning[QStringLiteral("zero")] = i18nc("@item:intable OpenType feature", "Slashed Zero");
    }
    return OTTagsMeaning.value(tag);
};

#endif
