//
// C++ Implementation: textprogression
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "textprogression.h"

TextProgression::TextProgression(QWidget * parent)
	:QWidget(parent)
{
	setupUi(this);
	
	connect(inlineBTT,SIGNAL(released()),this,SLOT(forwardStateChanged()));
	connect(inlineRTL,SIGNAL(released()),this,SLOT(forwardStateChanged()));
	connect(inlineLTR,SIGNAL(released()),this,SLOT(forwardStateChanged()));
	connect(inlineTTB,SIGNAL(released()),this,SLOT(forwardStateChanged()));
	connect(blockTTB,SIGNAL(released()),this,SLOT(forwardStateChanged()));
	connect(blockRTL,SIGNAL(released()),this,SLOT(forwardStateChanged()));
	connect(blockLTR,SIGNAL(released()),this,SLOT(forwardStateChanged()));
}

TextProgression::Progression TextProgression::inBlock()
{
	if(blockTTB->isChecked())
		return BLOCK_TTB;
	else if(blockRTL->isChecked())
		return BLOCK_RTL;
	else if(blockLTR->isChecked())
		return BLOCK_LTR;
	
	return UNDEFINED;
}

TextProgression::Progression TextProgression::inLine()
{
	if(inlineLTR->isChecked())
		return INLINE_LTR;
	else if(inlineRTL->isChecked())
		return INLINE_RTL;
	else if(inlineTTB->isChecked())
		return INLINE_TTB;
	else if(inlineBTT->isChecked())
		return INLINE_BTT;
	
	return UNDEFINED;
}

void TextProgression::forwardStateChanged( )
{
	emit stateChanged();
}
