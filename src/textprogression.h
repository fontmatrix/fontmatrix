//
// C++ Interface: textprogression
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TEXTPROGRESSION_H
#define TEXTPROGRESSION_H

#include "ui_textprogression.h"

class TextProgression : public QWidget, private Ui::TextProgressionWidget
{
	Q_OBJECT
	public:
		enum Progression {INLINE_LTR, INLINE_RTL, INLINE_TTB, INLINE_BTT, BLOCK_TTB, BLOCK_LTR, BLOCK_RTL, UNDEFINED};
		
		TextProgression(QWidget *parent);
		
		Progression inBlock();
		Progression inLine();
		
		static TextProgression* getInstance(){return instance;}
	private:
		static TextProgression *instance;
	signals:
		void stateChanged();
	private slots:
		void forwardStateChanged();
};

#endif
