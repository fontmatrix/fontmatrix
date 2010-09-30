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

#include "floatingwidgettoolbar.h"
#include "ui_floatingwidgettoolbar.h"

#include "floatingwidget.h"

#include <QMenu>
#include <QAction>

FloatingWidgetToolBar::FloatingWidgetToolBar(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::FloatingWidgetToolBar),
		noClose(false),
		isDetached(false)
{
	ui->setupUi(this);

	menu = new QMenu(this);
	setupMenu();
	ui->toolButton->setMenu(menu);

}

FloatingWidgetToolBar::~FloatingWidgetToolBar()
{
	delete ui;
}

void FloatingWidgetToolBar::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}


void FloatingWidgetToolBar::setupMenu()
{
	menu->clear();

	if(isDetached)
	{
		if(!noClose)
		{
			closeAction = new QAction(tr("Close"), menu);
			menu->addAction(closeAction);
			connect(closeAction, SIGNAL(triggered()), this, SIGNAL(Close()));
		}
		hideAction = new QAction(tr("Hide"), menu);
		menu->addAction(hideAction);
		connect(hideAction, SIGNAL(triggered()), this, SIGNAL(Hide()));
	}
	printAction = new QAction(tr("Print"), menu);
	menu->addAction(printAction);
	connect(printAction, SIGNAL(triggered()), this, SIGNAL(Print()));
	if(!isDetached)
	{
		detachAction = new QAction(tr("Detach"), menu);
		menu->addAction(detachAction);
		connect(detachAction, SIGNAL(triggered()), this, SLOT(setDetached()));
	}


}

void FloatingWidgetToolBar::setDetached()
{
	isDetached = true;
	setupMenu();
	emit Detach();
}

void FloatingWidgetToolBar::setNoClose(bool c)
{
	noClose = c;
	setupMenu();
}
