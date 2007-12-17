//
// C++ Interface: fontbook
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FONTBOOK_H
#define FONTBOOK_H

#include <QDomDocument>

/**
	@author Pierre Marchand <pierremarc@oep-h.com>

	It’s time to write something more "definitive" :)
*/
class FontBook
{
	public:
		FontBook();

		~FontBook();
		void doBook();
	private:
		void doBookFromTemplate(QDomDocument aTemplate);
};

#endif
