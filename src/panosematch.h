//
// C++ Interface: panosematch
//
// Description:
//
//
// Author: Riku Leino <riku.leino@gmail.com>, (C) 2008
//         David L. Wagner, International Business Machines Corp., 2002
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PANOSEMATCH_H
#define PANOSEMATCH_H

class PanoseMatch {

public:
	/** Init the matcher with the Panose string for the selected font */
	PanoseMatch(const QString &selected);
	
	PanoseMatch(){}
	~PanoseMatch(){}

	void setAttributes(const QString &attrs);
	
	/** Determine the difference between the selected font and the other font. */
	int diff (const QString &other) const;

private:
	QList<int> attributes;

	QList<int> parse(const QString &panoseString) const;

	/** Calculate the square of the difference between two Panose values. */
	static int calcdiffm (int, int, int);

	/** Calculate the difference between two discrete Panose values. */
	static int calcdiffd (int, int);

	/** Calculate the difference between two Serif Style values. */
	static int calcdiffss (int, int);

	/** Calculate the difference between two Letterform values. */
	static int calcdifflf (int, int);

	enum PanoseKey
	{
		FamilyType = 0,
		SerifStyle,
		Weight,
		Proportion,
		Contrast,
		StrokeVariation,
		ArmStyle,
		Letterform,
		Midline,
		XHeight,
		InvalidPK = 9999999
		};
};

class FontItem;

/// An helper class for Panose matching
class PanoseMatchFont : private PanoseMatch
{
	PanoseMatchFont(){}
	~PanoseMatchFont(){}
	public:
		static QList<FontItem*> similar(FontItem* ref, int treshold);
		
};


#endif // PANOSEMATCH_H
