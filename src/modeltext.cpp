//
// C++ Implementation: modeltext
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "modeltext.h"

#include <QMimeData>
#include <QDebug>


ModelText::ModelText(QWidget * parent)
	:QTextEdit(parent)
{
}

bool ModelText::canInsertFromMimeData(const QMimeData * source) const
{
	if(source->hasFormat( "application/x-qabstractitemmodeldatalist" ))
	{
		return true;
	}
	else
		return QTextEdit::canInsertFromMimeData(source);
}

void ModelText::insertFromMimeData(const QMimeData * source)
{
	if(source->hasFormat( "application/x-qabstractitemmodeldatalist" ))
	{
		emit insertContent();
	}
	else
		QTextEdit::insertFromMimeData(source);
}
