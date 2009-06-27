//
// C++ Interface: fminfodisplay
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FMINFODISPLAY_H
#define FMINFODISPLAY_H

#include <QString>

/**
A processing class generating XHTML to be displayed in Info tab.

	@author Pierre Marchand <pierremarc@oep-h.com>
*/

class FontItem;
class FMInfoDisplay
{
		FMInfoDisplay(){}
		QString html;
		
		QString writeFsType(FontItem * font);
		QString writeSVGPreview(FontItem * font);
		QString writeOrderedInfo(FontItem * font);
		QString writePanose(FontItem * font);
		QString writeLangOS2(FontItem * font);
		
		QString url2href(QString value);
		QString xhtmlifies(const QString& value);
		
	public:
		FMInfoDisplay(FontItem * font);
		~FMInfoDisplay();
		
		QString getHtml();

};

#endif
