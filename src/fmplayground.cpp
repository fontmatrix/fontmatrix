//
// C++ Implementation: fmplayground
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmplayground.h"


FMPlayGround::FMPlayGround(QWidget *parent)
	:QGraphicsView(parent)
{
	setInteractive(true);
	setDragMode(RubberBandDrag);
	setRenderHint(QPainter::Antialiasing);
}

FMPlayGround::~ FMPlayGround()
{
}

