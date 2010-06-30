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

#ifndef FLOATINGWIDGET_H
#define FLOATINGWIDGET_H

#include <QWidget>
#include <QString>

class FloatingWidget : public QWidget
{
	Q_OBJECT

	explicit FloatingWidget(QWidget *parent = 0){}
public:
	explicit FloatingWidget(const QString &f, const QString& typ, QWidget *parent = 0);
	~FloatingWidget();

	QString getActionName()const{return actionName;}

private:
	QString fName;
	QString fType;
	QString actionName;
	QString wTitle;

protected:
	virtual bool event( QEvent * e );

signals:
	void visibilityChange();
	void detached();

public slots:
	void activate(bool a);
	void detach();

};

#endif // FLOATINGWIDGET_H
