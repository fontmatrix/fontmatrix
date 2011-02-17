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

#ifndef FLOATINGWIDGETTOOLBAR_H
#define FLOATINGWIDGETTOOLBAR_H

#include <QWidget>

class QMenu;
class QAction;

namespace Ui {
    class FloatingWidgetToolBar;
}

class FloatingWidgetToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingWidgetToolBar(QWidget *parent = 0);
    ~FloatingWidgetToolBar();

    void setNoClose(bool c);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FloatingWidgetToolBar *ui;

    bool noClose;
    bool isDetached;

    void setupMenu();

public slots:
    void setDetached();

signals:
    void Close();
    void Hide();
    void Print();
    void Detach();
};

#endif // FLOATINGWIDGETTOOLBAR_H
