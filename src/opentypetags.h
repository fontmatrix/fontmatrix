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

#ifndef OTTMEAN_H
#define OTTMEAN_H

#include <QMap>


QString OTTagMeans ( QString tag )
{
	static QMap<QString,QString> OTTagsMeaning;
	if ( OTTagsMeaning.isEmpty() )
	{
		OTTagsMeaning["aalt"]= QObject::tr("Access All Alternates");
		OTTagsMeaning["abvf"]= QObject::tr("Above-Base Forms");
		OTTagsMeaning["abvm"]= QObject::tr("Above-Base Mark Positioning");
		OTTagsMeaning["abvs"]= QObject::tr("Above-Base Substitutions");
		OTTagsMeaning["afrc"]= QObject::tr("Alternative Fractions");
		OTTagsMeaning["akhn"]= QObject::tr("Akhands");
		OTTagsMeaning["blwf"]= QObject::tr("Below-Base Forms");
		OTTagsMeaning["blwm"]= QObject::tr("Below-Base Mark Positioning");
		OTTagsMeaning["blws"]= QObject::tr("Below-Base Substitutions");
		OTTagsMeaning["c2pc"]= QObject::tr("Petite Capitals From Capitals");
		OTTagsMeaning["c2sc"]= QObject::tr("Small Capitals From Capitals");
		OTTagsMeaning["calt"]= QObject::tr("Contextual Alternates");
		OTTagsMeaning["case"]= QObject::tr("Case-Sensitive Forms");
		OTTagsMeaning["ccmp"]= QObject::tr("Glyph Composition/Decomposition");
		OTTagsMeaning["clig"]= QObject::tr("Contextual Ligatures");
		OTTagsMeaning["cjct"]= QObject::tr("Conjunct Forms");
		OTTagsMeaning["cpsp"]= QObject::tr("Capital Spacing");
		OTTagsMeaning["cswh"]= QObject::tr("Contextual Swash");
		OTTagsMeaning["curs"]= QObject::tr("Cursive Positioning");
		OTTagsMeaning["dflt"]= QObject::tr("Default Processing");
		OTTagsMeaning["dist"]= QObject::tr("Distances");
		OTTagsMeaning["dlig"]= QObject::tr("Discretionary Ligatures");
		OTTagsMeaning["dnom"]= QObject::tr("Denominators");
		OTTagsMeaning["expt"]= QObject::tr("Expert Forms");
		OTTagsMeaning["falt"]= QObject::tr("Final glyph Alternates");
		OTTagsMeaning["fin2"]= QObject::tr("Terminal Forms #2");
		OTTagsMeaning["fin3"]= QObject::tr("Terminal Forms #3");
		OTTagsMeaning["fina"]= QObject::tr("Terminal Forms");
		OTTagsMeaning["frac"]= QObject::tr("Fractions");
		OTTagsMeaning["fwid"]= QObject::tr("Full Width");
		OTTagsMeaning["half"]= QObject::tr("Half Forms");
		OTTagsMeaning["haln"]= QObject::tr("Halant Forms");
		OTTagsMeaning["halt"]= QObject::tr("Alternate Half Width");
		OTTagsMeaning["hist"]= QObject::tr("Historical Forms");
		OTTagsMeaning["hkna"]= QObject::tr("Horizontal Kana Alternates");
		OTTagsMeaning["hlig"]= QObject::tr("Historical Ligatures");
		OTTagsMeaning["hngl"]= QObject::tr("Hangul");
		OTTagsMeaning["hojo"]= QObject::tr("Hojo Kanji Forms (JIS x 212-1990 Kanji Forms)");
		OTTagsMeaning["hwid"]= QObject::tr("Half Width");
		OTTagsMeaning["init"]= QObject::tr("Initial Forms");
		OTTagsMeaning["isol"]= QObject::tr("Isolated Forms");
		OTTagsMeaning["ital"]= QObject::tr("Italics");
		OTTagsMeaning["jalt"]= QObject::tr("Justification Alternatives");
		OTTagsMeaning["jp78"]= QObject::tr("JIS78 Forms");
		OTTagsMeaning["jp83"]= QObject::tr("JIS83 Forms");
		OTTagsMeaning["jp90"]= QObject::tr("JIS90 Forms");
		OTTagsMeaning["jp04"]= QObject::tr("JIS2004 Forms");
		OTTagsMeaning["kern"]= QObject::tr("Kerning");
		OTTagsMeaning["lfbd"]= QObject::tr("Left Bounds");
		OTTagsMeaning["liga"]= QObject::tr("Standard Ligatures");
		OTTagsMeaning["ljmo"]= QObject::tr("Leading Jamo Forms");
		OTTagsMeaning["lnum"]= QObject::tr("Lining Figures");
		OTTagsMeaning["locl"]= QObject::tr("Localized Forms");
		OTTagsMeaning["mark"]= QObject::tr("Mark Positioning");
		OTTagsMeaning["med2"]= QObject::tr("Medial Forms #2");
		OTTagsMeaning["medi"]= QObject::tr("Medial Forms");
		OTTagsMeaning["mgrk"]= QObject::tr("Mathematical Greek");
		OTTagsMeaning["mkmk"]= QObject::tr("Mark to Mark Positioning");
		OTTagsMeaning["mset"]= QObject::tr("Mark Positioning via Substitution");
		OTTagsMeaning["nalt"]= QObject::tr("Alternate Annotation Forms");
		OTTagsMeaning["nlck"]= QObject::tr("NLC Kanji Forms");
		OTTagsMeaning["nukt"]= QObject::tr("Nukta Forms");
		OTTagsMeaning["numr"]= QObject::tr("Numerators");
		OTTagsMeaning["onum"]= QObject::tr("Old Style Figures");
		OTTagsMeaning["opbd"]= QObject::tr("Optical Bounds");
		OTTagsMeaning["ordn"]= QObject::tr("Ordinals");
		OTTagsMeaning["ornm"]= QObject::tr("Ornaments");
		OTTagsMeaning["palt"]= QObject::tr("Proportional Alternate Width");
		OTTagsMeaning["pcap"]= QObject::tr("Petite Capitals");
		OTTagsMeaning["pnum"]= QObject::tr("Proportional Figures");
		OTTagsMeaning["pref"]= QObject::tr("Pre-base Forms");
		OTTagsMeaning["pres"]= QObject::tr("Pre-base Substitutions");
		OTTagsMeaning["pstf"]= QObject::tr("Post-base Forms");
		OTTagsMeaning["psts"]= QObject::tr("Post-base Substitutions");
		OTTagsMeaning["pwid"]= QObject::tr("Proportional Widths");
		OTTagsMeaning["qwid"]= QObject::tr("Quarter Widths");
		OTTagsMeaning["rand"]= QObject::tr("Randomize");
		OTTagsMeaning["rkrf"]= QObject::tr("Rakar Forms");
		OTTagsMeaning["rlig"]= QObject::tr("Required Ligatures");
		OTTagsMeaning["rphf"]= QObject::tr("Reph Form");
		OTTagsMeaning["rtbd"]= QObject::tr("Right Bounds");
		OTTagsMeaning["rtla"]= QObject::tr("Right-To-Left Alternates");
		OTTagsMeaning["ruby"]= QObject::tr("Ruby Notation Forms");
		OTTagsMeaning["salt"]= QObject::tr("Stylistic Alternates");
		OTTagsMeaning["sinf"]= QObject::tr("Scientific Inferiors");
		OTTagsMeaning["size"]= QObject::tr("Optical Size");
		OTTagsMeaning["smcp"]= QObject::tr("Small Capitals");
		OTTagsMeaning["smpl"]= QObject::tr("Simplified Forms");
		OTTagsMeaning["ss01"]= QObject::tr("Sylistic Set 1");
		OTTagsMeaning["ss02"]= QObject::tr("Sylistic Set 2");
		OTTagsMeaning["ss03"]= QObject::tr("Sylistic Set 3");
		OTTagsMeaning["ss04"]= QObject::tr("Sylistic Set 4");
		OTTagsMeaning["ss05"]= QObject::tr("Sylistic Set 5");
		OTTagsMeaning["ss06"]= QObject::tr("Sylistic Set 6");
		OTTagsMeaning["ss07"]= QObject::tr("Sylistic Set 7");
		OTTagsMeaning["ss08"]= QObject::tr("Sylistic Set 8");
		OTTagsMeaning["ss09"]= QObject::tr("Sylistic Set 9");
		OTTagsMeaning["ss10"]= QObject::tr("Sylistic Set 10");
		OTTagsMeaning["ss11"]= QObject::tr("Sylistic Set 11");
		OTTagsMeaning["ss12"]= QObject::tr("Sylistic Set 12");
		OTTagsMeaning["ss13"]= QObject::tr("Sylistic Set 13");
		OTTagsMeaning["ss14"]= QObject::tr("Sylistic Set 14");
		OTTagsMeaning["ss15"]= QObject::tr("Sylistic Set 15");
		OTTagsMeaning["ss16"]= QObject::tr("Sylistic Set 16");
		OTTagsMeaning["ss17"]= QObject::tr("Sylistic Set 17");
		OTTagsMeaning["ss18"]= QObject::tr("Sylistic Set 18");
		OTTagsMeaning["ss19"]= QObject::tr("Sylistic Set 19");
		OTTagsMeaning["ss20"]= QObject::tr("Sylistic Set 20");
		OTTagsMeaning["subs"]= QObject::tr("Subscript");
		OTTagsMeaning["sups"]= QObject::tr("Superscript");
		OTTagsMeaning["swsh"]= QObject::tr("Swash");
		OTTagsMeaning["titl"]= QObject::tr("Titling");
		OTTagsMeaning["tjmo"]= QObject::tr("Trailing Jamo Forms");
		OTTagsMeaning["tnam"]= QObject::tr("Traditional Name Forms");
		OTTagsMeaning["tnum"]= QObject::tr("Tabular Figures");
		OTTagsMeaning["trad"]= QObject::tr("Traditional Forms");
		OTTagsMeaning["twid"]= QObject::tr("Third Widths");
		OTTagsMeaning["unic"]= QObject::tr("Unicase");
		OTTagsMeaning["valt"]= QObject::tr("Alternate Vertical Metrics");
		OTTagsMeaning["vatu"]= QObject::tr("Vattu Variants");
		OTTagsMeaning["vert"]= QObject::tr("Vertical Writing");
		OTTagsMeaning["vhal"]= QObject::tr("Alternate Vertical Half Metrics");
		OTTagsMeaning["vjmo"]= QObject::tr("Vowel Jamo Forms");
		OTTagsMeaning["vkna"]= QObject::tr("Vertical Kana Alternates");
		OTTagsMeaning["vkrn"]= QObject::tr("Vertical Kerning");
		OTTagsMeaning["vpal"]= QObject::tr("Proportional Alternate Vertical Metrics");
		OTTagsMeaning["vrt2"]= QObject::tr("Vertical Rotation");
		OTTagsMeaning["zero"]= QObject::tr("Slashed Zero");
	}
	return OTTagsMeaning.value(tag);
};

#endif
