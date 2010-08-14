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

#ifndef ACTIVATIONWIDGET_H
#define ACTIVATIONWIDGET_H

#include "floatingwidget.h"

namespace Ui {
	class ActivationWidget;
}

class ActivationWidgetItem;

class ActivationWidget : public FloatingWidget
{
	Q_OBJECT

public:
	static const QString Name;
	explicit ActivationWidget(const QString& familyName, QWidget *parent = 0);
	~ActivationWidget();

protected:
	void changeEvent(QEvent *e);

private:
	const QString family;
	Ui::ActivationWidget *ui;

	QList<ActivationWidgetItem*> items;
	void activateAll(bool c);

private slots:
	void slotActivate();
	void slotDeactivate();

signals:
	void familyStateChanged();
};

#endif // ACTIVATIONWIDGET_H
