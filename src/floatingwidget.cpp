/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
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

#include "floatingwidget.h"
#include "floatingwidgetsregister.h"
#include "typotek.h"
#include "fmfontdb.h"
#include "fontitem.h"

FloatingWidget::FloatingWidget(const QString &f, const QString& typ, QWidget *parent) :
		QWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	QString fn(FMFontDb::DB()->Font(f)->fancyName());
	actionName =  QString("[%1]").arg(typ) + QString(" ") + fn;
	wTitle =  fn + QString(" - Fontmatrix");
	setProperty("windowTitle", wTitle);
	FloatingWidgetsRegister::Register(this, f, typ);
}

FloatingWidget::~FloatingWidget()
{
}


bool FloatingWidget::event(QEvent *e)
{
//	if(windowTitle().isEmpty())
//	{
//		QWidget::setWindowTitle(wTitle);
//	}
	if((e->type() == QEvent::Show) || (e->type() == QEvent::Hide))
		emit visibilityChange();

	return QWidget::event(e);
}

void FloatingWidget::activate(bool a)
{
	if(a)
	{
		if(!isVisible())
			setVisible(true);
		raise();
	}
	else
		hide();
}

