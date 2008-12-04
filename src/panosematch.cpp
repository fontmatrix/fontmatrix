//
// C++ Implementation: panosematch
//
// Description:
//
//
// Author: Riku Leino <riku.leino@gmail.com>, (C) 2008
//         David L. Wagner, International Business Machines Corp., 2002
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
// http://www.gnu.org/copyleft/lesser.html
//

#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "panosematch.h"

#include <QList>

/**
 * @param selected colon separated list of the ten panose attributes (2:2:2:2:2:2:2:2:2:2)
 */
PanoseMatch::PanoseMatch(const QString &selected)
{
	attributes = parse(selected);
}

void PanoseMatch::setAttributes(const QString & attrs)
{
	attributes = parse(attrs);
}

/**
 * Returns the difference between the significant fields of the current Panose
 * object and the given one.  The contract for this method says that an exact
 * match will produce the lowest possible integer value, and fonts that differ
 * will produce larger ones.
 *
 * In fact, this implementation produces 0 for an exact match, and calculates
 * the sum of the squares of the differences between the significant fields.
 *
 * This implementation is completely wrong, however.  Some things are
 * continuous (such as weight) but some are discrete (like family type), so we
 * really should use the sum-of-the-squares thing for continuous values, and
 * have sort of large penalty for discrete ones.
 *
 * @param other colon separated list of the ten panose attributes (2:2:2:2:2:2:2:2:2:2)
 */
int PanoseMatch::diff (const QString &other) const
{
	QList<int> attrOther = parse(other);
	if (attrOther.isEmpty() || attributes.isEmpty())
		return 10000;

	int result = 0;
	//
	// If they aren't in the same family, then it really makes no sense to
	// compare them at all, so throw in some big penalty.
	//
	if (attributes[FamilyType] != 0 &&
	    attrOther[FamilyType] != 0 &&
        attributes[FamilyType] != attrOther[FamilyType])
	    	result = 5000;
	//
	// It's debatable whether the Serif_Style number is monotonic, but we'll
	// treat it as such in this implementation.
	//
	result += calcdiffm (attributes[Weight],
						attrOther[Weight], 12);
	result += calcdiffm (attributes[Contrast],
						attrOther[Contrast], 10);
	result += calcdiffm (attributes[StrokeVariation],
						attrOther[StrokeVariation], 9);
	//
	// The Proportion, Arm_Style, Letterform, Midline and X_Height values aren't
	// monotonically increasing values, so we shouldn't give much weight to
	// to these values (although we could probably do some rearranging to get the
	// Proportion values to be approximately monotonic).
	//
	result += calcdiffss (attributes[SerifStyle],
	                      attrOther[SerifStyle]);
	result += calcdiffd (attributes[Proportion],
	                     attrOther[Proportion]);
	result += calcdiffd (attributes[ArmStyle],
	                     attrOther[ArmStyle]);
	result += calcdifflf (attributes[Letterform],
	                      attrOther[Letterform]);
	result += calcdiffd (attributes[Midline],
	                     attrOther[Midline]);
	result += calcdiffd (attributes[XHeight],
	                     attrOther[XHeight]);
	return result;
}

QList<int> PanoseMatch::parse(const QString &panoseString) const
{
	QStringList pl(panoseString.split(":"));
	QList<int> l;
	if(pl.count() == 10) {
		foreach(QString s, pl) {
			l << s.toInt();
		}
	}
	return l;
}

/**
 * Calculate the square of the difference between two monotonic Panose values
 * (as integers), with a weight applied.  If either value is 0 (xxx_ANY), then
 * value returned is zero.  If one value is 1 (xxx_NO_FIT), then they can't
 * be classified, so we give a penalty.  This method will return some
 * value between 0 and 1000.  This method only makes sense when the values
 * are in a continuous range.
 *
 * @param p1      first Panose value
 * @param p2      second Panose value
 * @param weight  the number of Panose values in this enumeration
 */
int PanoseMatch::calcdiffm (int p1, int p2, int weight)
{
	//
	// If they can have any value, then they automatically match
	//
	if (p1 == 0 || p2 == 0)
		return 0;
	//
	// If one (but not both) value is xxx_NO_FIT, then they can't match
	//
	if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
		return 1000;
	//
	// Now return some number between 0 and 1000 (the value "weight-3" is the
	// maximum difference between p1 and p2).
	//
	return 1000 * (p1-p2)*(p1-p2) / (weight-3)*(weight-3);
}


/**
 * Calculate the square of the difference between two discrete (non-monotonic)
 * Panose values (as integers).  If either value is 0 (xxx_ANY), then value
 * returned is zero.  If one value is 1 (xxx_NO_FIT), then they can't can't be
 * classified, so we give a penalty.  This method will return some
 * value between 0 and 100.
 *
 * @param p1      first Panose value
 * @param p2      second Panose value
 */
int PanoseMatch::calcdiffd (int p1, int p2)
{
	//
	// If they can have any value, then they automatically match
	//
	if (p1 == 0 || p2 == 0 || p1 == p2)
		return 0;
	//
	// If one (but not both) value is xxx_NO_FIT, then they can't match
	//
	if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
		return 1000;
	//
	// Otherwise, we really can't compare the values sensible, so just return
	// some small penalty.
	//
		return 100;
}


/**
 * Calculate the difference between two serif styles; we treat differences
 * any two serifs or sans-serifs as small, and the difference between any
 * serif and any sans-serif as large.
 */
int PanoseMatch::calcdiffss (int p1, int p2)
{
  //
  // If they can have any value, then they automatically match
  //
  if (p1 == 0 || p2 == 0 || p1 == p2)
    return 0;
  //
  // If one (but not both) value is xxx_NO_FIT, then they can't match
  //
  if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
    return 1000;
  //
  // Otherwise, check to see if they are serif or sans-serif
  // These ints are from fmfontstrings.cpp (needs something more clever)
  // TODO create a proper panose class
  if (p1 >= 11 && p1 <= 13 &&
      p2 >= 11 && p2 <= 13)
    return 10;
  if ((p1 < 11 || p1 > 13) &&
      (p2 < 11 || p2 > 13))
    return 10;
  else
    return 100;
}


/**
 * Calculate the difference between two letter forms; we treat differences
 * any two normals or obliques as small, and the difference between any
 * normal and any oblique as large.
 */
int PanoseMatch::calcdifflf (int p1, int p2)
{
  //
  // If they can have any value, then they automatically match
  //
  if (p1 == 0 || p2 == 0 || p1 == p2)
    return 0;
  //
  // If one (but not both) value is xxx_NO_FIT, then they can't match
  //
  if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
    return 1000;
  //
  // Otherwise, check to see if they are normal or oblique
  // 9 = Oblique/Contact from fmfontstrings.cpp
  if ((p1 <  9 && p2 <  9) ||
      (p1 >= 9 && p2 >= 9))
    return 10;
  else
    return 100;
}

/// PanoseMatchFont

QList< FontItem * > PanoseMatchFont::similar(FontItem * ref, int treshold)
{
	if((!ref) || (!treshold))
		return QList< FontItem * >();
	if(ref->panose().isEmpty())
		return QList< FontItem * >();

	PanoseMatchFont * pm(new PanoseMatchFont);
	pm->setAttributes(ref->panose());

	QList<FontItem*> all(FMFontDb::DB()->AllFonts());
	QList<FontDBResult> dbresult( FMFontDb::DB()->getInfo(all, FMFontDb::Panose) );
	QList<FontItem*> fil;
	for(int i(0); i < dbresult.count() ; ++i)
	{
		if( pm->diff(dbresult[i].second) < treshold )
			fil << dbresult[i].first;
	}
	delete  pm;
	return fil;
}

