//
// C++ Interface: verticallabel
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMVERTICALBUTTON_H
#define FMVERTICALBUTTON_H

#include <QToolButton>
#include <QFont>

class FMVerticalButton : public QToolButton
{
	public:
		FMVerticalButton ( QWidget * parent );
		~FMVerticalButton();
	protected:
		bool event ( QEvent * event )  ;

	private:
		QString m_text;
		QFont m_font;
		
};

#endif

