/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
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

#ifndef FMFLOATINGPREVIEW_H
#define FMFLOATINGPREVIEW_H

#include <QWidget>
#include <QRect>

class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QFrame;

class FontItem;

class FMFloatingMenu : public QWidget
{
	Q_OBJECT
public:
	FMFloatingMenu(QWidget * parent, FontItem * item);
	void childrenVisible(bool v);

//protected:
//	void enterEvent(QEvent * e);
//	void leaveEvent(QEvent *e);

private:
	FontItem * fontItem;
	QPushButton * closeButton;
	QPushButton * actButton;
	QFrame * line;
	QHBoxLayout * menuLayout;
	QLabel * fontName;


private slots:
	void forwardCloseClicked();
	void activateFont();

signals:
	void closeClicked();
};

class FontItem;
class FMFloatingPreview : public QWidget
{
	FMFloatingPreview(QWidget * parent, FontItem * item);
public:
	~FMFloatingPreview();
	static void create(FontItem* item, QRect pos=QRect());

protected:
	void mousePressEvent(QMouseEvent * e);
	void mouseReleaseEvent(QMouseEvent * e);
	void mouseMoveEvent(QMouseEvent * e);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private:
	bool hasMouseGrab;
	QPoint refPoint;
	FMFloatingMenu * menuWidget;
	QVBoxLayout * mainLayout;
	QLabel * previewLabel;

	bool canTransparent();

};

#endif // FMFLOATINGPREVIEW_H
