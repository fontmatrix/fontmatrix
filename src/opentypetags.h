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
		OTTagsMeaning["aalt"]="Access All Alternates";
		OTTagsMeaning["abvf"]="Above-Base Forms";
		OTTagsMeaning["abvm"]="Above-Base Mark Positioning";
		OTTagsMeaning["abvs"]="Above-Base Substitutions";
		OTTagsMeaning["afrc"]="Alternative Fractions";
		OTTagsMeaning["akhn"]="Akhands";
		OTTagsMeaning["blwf"]="Below-Base Forms";
		OTTagsMeaning["blwm"]="Below-Base Mark Positioning";
		OTTagsMeaning["blws"]="Below-Base Substitutions";
		OTTagsMeaning["c2pc"]="Petite Capitals From Capitals";
		OTTagsMeaning["c2sc"]="Small Capitals From Capitals";
		OTTagsMeaning["calt"]="Contextual Alternates";
		OTTagsMeaning["case"]="Case-Sensitive Forms";
		OTTagsMeaning["ccmp"]="Glyph Composition/Decomposition";
		OTTagsMeaning["clig"]="Contextual Ligatures";
		OTTagsMeaning["cjct"]="Conjunct Forms";
		OTTagsMeaning["cpsp"]="Capital Spacing";
		OTTagsMeaning["cswh"]="Contextual Swash";
		OTTagsMeaning["curs"]="Cursive Positioning";
		OTTagsMeaning["dflt"]="Default Processing";
		OTTagsMeaning["dist"]="Distances";
		OTTagsMeaning["dlig"]="Discretionary Ligatures";
		OTTagsMeaning["dnom"]="Denominators";
		OTTagsMeaning["expt"]="Expert Forms";
		OTTagsMeaning["falt"]="Final glyph Alternates";
		OTTagsMeaning["fin2"]="Terminal Forms #2";
		OTTagsMeaning["fin3"]="Terminal Forms #3";
		OTTagsMeaning["fina"]="Terminal Forms";
		OTTagsMeaning["frac"]="Fractions";
		OTTagsMeaning["fwid"]="Full Width";
		OTTagsMeaning["half"]="Half Forms";
		OTTagsMeaning["haln"]="Halant Forms";
		OTTagsMeaning["halt"]="Alternate Half Width";
		OTTagsMeaning["hist"]="Historical Forms";
		OTTagsMeaning["hkna"]="Horizontal Kana Alternates";
		OTTagsMeaning["hlig"]="Historical Ligatures";
		OTTagsMeaning["hngl"]="Hangul";
		OTTagsMeaning["hojo"]="Hojo Kanji Forms (JIS x 212-1990 Kanji Forms)";
		OTTagsMeaning["hwid"]="Half Width";
		OTTagsMeaning["init"]="Initial Forms";
		OTTagsMeaning["isol"]="Isolated Forms";
		OTTagsMeaning["ital"]="Italics";
		OTTagsMeaning["jalt"]="Justification Alternatives";
		OTTagsMeaning["jp78"]="JIS78 Forms";
		OTTagsMeaning["jp83"]="JIS83 Forms";
		OTTagsMeaning["jp90"]="JIS90 Forms";
		OTTagsMeaning["jp04"]="JIS2004 Forms";
		OTTagsMeaning["kern"]="Kerning";
		OTTagsMeaning["lfbd"]="Left Bounds";
		OTTagsMeaning["liga"]="Standard Ligatures";
		OTTagsMeaning["ljmo"]="Leading Jamo Forms";
		OTTagsMeaning["lnum"]="Lining Figures";
		OTTagsMeaning["locl"]="Localized Forms";
		OTTagsMeaning["mark"]="Mark Positioning";
		OTTagsMeaning["med2"]="Medial Forms #2";
		OTTagsMeaning["medi"]="Medial Forms";
		OTTagsMeaning["mgrk"]="Mathematical Greek";
		OTTagsMeaning["mkmk"]="Mark to Mark Positioning";
		OTTagsMeaning["mset"]="Mark Positioning via Substitution";
		OTTagsMeaning["nalt"]="Alternate Annotation Forms";
		OTTagsMeaning["nlck"]="NLC Kanji Forms";
		OTTagsMeaning["nukt"]="Nukta Forms";
		OTTagsMeaning["numr"]="Numerators";
		OTTagsMeaning["onum"]="Old Style Figures";
		OTTagsMeaning["opbd"]="Optical Bounds";
		OTTagsMeaning["ordn"]="Ordinals";
		OTTagsMeaning["ornm"]="Ornaments";
		OTTagsMeaning["palt"]="Proportional Alternate Width";
		OTTagsMeaning["pcap"]="Petite Capitals";
		OTTagsMeaning["pnum"]="Proportional Figures";
		OTTagsMeaning["pref"]="Pre-base Forms";
		OTTagsMeaning["pres"]="Pre-base Substitutions";
		OTTagsMeaning["pstf"]="Post-base Forms";
		OTTagsMeaning["psts"]="Post-base Substitutions";
		OTTagsMeaning["pwid"]="Proportional Widths";
		OTTagsMeaning["qwid"]="Quarter Widths";
		OTTagsMeaning["rand"]="Randomize";
		OTTagsMeaning["rkrf"]="Rakar Forms";
		OTTagsMeaning["rlig"]="Required Ligatures";
		OTTagsMeaning["rphf"]="Reph Form";
		OTTagsMeaning["rtbd"]="Right Bounds";
		OTTagsMeaning["rtla"]="Right-To-Left Alternates";
		OTTagsMeaning["ruby"]="Ruby Notation Forms";
		OTTagsMeaning["salt"]="Stylistic Alternates";
		OTTagsMeaning["sinf"]="Scientific Inferiors";
		OTTagsMeaning["size"]="Optical Size";
		OTTagsMeaning["smcp"]="Small Capitals";
		OTTagsMeaning["smpl"]="Simplified Forms";
		OTTagsMeaning["ss01"]="Sylistic Set 1";
		OTTagsMeaning["ss02"]="Sylistic Set 2";
		OTTagsMeaning["ss03"]="Sylistic Set 3";
		OTTagsMeaning["ss04"]="Sylistic Set 4";
		OTTagsMeaning["ss05"]="Sylistic Set 5";
		OTTagsMeaning["ss06"]="Sylistic Set 6";
		OTTagsMeaning["ss07"]="Sylistic Set 7";
		OTTagsMeaning["ss08"]="Sylistic Set 8";
		OTTagsMeaning["ss09"]="Sylistic Set 9";
		OTTagsMeaning["ss10"]="Sylistic Set 10";
		OTTagsMeaning["ss11"]="Sylistic Set 11";
		OTTagsMeaning["ss12"]="Sylistic Set 12";
		OTTagsMeaning["ss13"]="Sylistic Set 13";
		OTTagsMeaning["ss14"]="Sylistic Set 14";
		OTTagsMeaning["ss15"]="Sylistic Set 15";
		OTTagsMeaning["ss16"]="Sylistic Set 16";
		OTTagsMeaning["ss17"]="Sylistic Set 17";
		OTTagsMeaning["ss18"]="Sylistic Set 18";
		OTTagsMeaning["ss19"]="Sylistic Set 19";
		OTTagsMeaning["ss20"]="Sylistic Set 20";
		OTTagsMeaning["subs"]="Subscript";
		OTTagsMeaning["sups"]="Superscript";
		OTTagsMeaning["swsh"]="Swash";
		OTTagsMeaning["titl"]="Titling";
		OTTagsMeaning["tjmo"]="Trailing Jamo Forms";
		OTTagsMeaning["tnam"]="Traditional Name Forms";
		OTTagsMeaning["tnum"]="Tabular Figures";
		OTTagsMeaning["trad"]="Traditional Forms";
		OTTagsMeaning["twid"]="Third Widths";
		OTTagsMeaning["unic"]="Unicase";
		OTTagsMeaning["valt"]="Alternate Vertical Metrics";
		OTTagsMeaning["vatu"]="Vattu Variants";
		OTTagsMeaning["vert"]="Vertical Writing";
		OTTagsMeaning["vhal"]="Alternate Vertical Half Metrics";
		OTTagsMeaning["vjmo"]="Vowel Jamo Forms";
		OTTagsMeaning["vkna"]="Vertical Kana Alternates";
		OTTagsMeaning["vkrn"]="Vertical Kerning";
		OTTagsMeaning["vpal"]="Proportional Alternate Vertical Metrics";
		OTTagsMeaning["vrt2"]="Vertical Rotation";
		OTTagsMeaning["zero"]="Slashed Zero";
	}
	return OTTagsMeaning.value(tag);
};

#endif
