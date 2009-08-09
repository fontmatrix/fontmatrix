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

#include "fmfloatingpreview.h"
#include "typotek.h"
#include "fontitem.h"
#include "fmfontdb.h"
#include "fmactivate.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QPixmap>
#include <QSize>
#include <QMouseEvent>
#include <QCursor>

#if defined(Q_WS_X11)
#include <QX11Info>
#endif

#include <QDebug>

FMFloatingMenu::FMFloatingMenu(QWidget * parent, FontItem * item)
		:QWidget(parent), fontItem(item)
{
	menuLayout = new QHBoxLayout(this);
	menuLayout->setContentsMargins(3,0,3,0);

	QFont f(font());
	double fs(f.pointSizeF());
	f.setPointSizeF( 0.6 * fs);

	if(item)
	{
		QFont f2(f);
		f2.setBold(true);
		fontName = new QLabel(item->fancyName(), this);
		fontName->setFont(f2);
		menuLayout->addWidget(fontName, 0);
	}

	line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	menuLayout->addWidget(line, 2);

	if(item && !item->isActivated())
	{
		actButton = new QPushButton(tr("Activate"),this);
		actButton->setFont(f);
		menuLayout->addWidget(actButton, 0, Qt::AlignRight);
		connect(actButton, SIGNAL(clicked()),this,SLOT(activateFont()));
	}

	closeButton = new QPushButton(tr("close"), this);
	closeButton->setFont(f);
	menuLayout->addWidget(closeButton, 0, Qt::AlignRight);

	connect(closeButton, SIGNAL(clicked()), this, SLOT(forwardCloseClicked()));
}

//void FMFloatingMenu::enterEvent(QEvent *e)
//{
//	childrenVisible(true);
//	QWidget::enterEvent(e);
//}
//
//void FMFloatingMenu::leaveEvent(QEvent * e)
//{
//	childrenVisible(false);
//	QWidget::leaveEvent(e);
//}

void FMFloatingMenu::childrenVisible(bool v)
{
	foreach(QWidget *w, findChildren<QWidget*>())
	{
		w->setVisible(v);
	}
}

void FMFloatingMenu::forwardCloseClicked()
{
	emit closeClicked();
}

void FMFloatingMenu::activateFont()
{
	QList<FontItem*> fl;
	fl << fontItem;
	FMActivate::getInstance()->activate(fl, true);
	menuLayout->removeWidget(actButton);
	delete actButton;
}

FMFloatingPreview::FMFloatingPreview(QWidget * parent, FontItem * item)
		:QWidget(parent)
{
	hasMouseGrab = false;
	setWindowFlags( Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	if(canTransparent())
		setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_DeleteOnClose, true);


	mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	menuWidget = new FMFloatingMenu(this, item);
	previewLabel = new QLabel(this);
	mainLayout->addWidget(menuWidget, Qt::AlignHCenter);
	mainLayout->addWidget(previewLabel, Qt::AlignHCenter);


	connect(menuWidget, SIGNAL(closeClicked()), this, SLOT(close()));
}

FMFloatingPreview::~FMFloatingPreview()
{

}


void FMFloatingPreview::create(FontItem *item, QRect rect)
{
	FMFloatingPreview * p(new FMFloatingPreview(typotek::getInstance(), item));
	QColor bgC;
	if(p->canTransparent())
		bgC = QColor(Qt::transparent);
	else
		bgC = QColor(Qt::white);
	QPixmap preview = item->oneLinePreviewPixmap(typotek::getInstance()->word(item), Qt::black, bgC, rect.width());

	QRect r(preview.rect());
	QPoint delta(r.width()/2, r.height()/2);
	p->previewLabel->setGeometry(r);
	p->previewLabel->setPixmap(preview);

	QRect r2(QPoint(rect.x() - (r.width()/2),
			QCursor::pos().y() - (p->menuWidget->geometry().height()/2) ),
		 QSize( r.width(),
			r.height() + p->menuWidget->geometry().height()));

	p->setGeometry(r2);
	p->show();
	p->setFocus(Qt::OtherFocusReason);
	p->grabMouse();
	QApplication::setOverrideCursor(Qt::SizeAllCursor);
	p->hasMouseGrab = true;
	p->refPoint = QCursor::pos() -  p->geometry().topLeft();
}

void FMFloatingPreview::mousePressEvent(QMouseEvent * e)
{
	if (!(e->buttons() & Qt::LeftButton))
		return;
	refPoint = e->globalPos() - geometry().topLeft();
	QApplication::setOverrideCursor(Qt::SizeAllCursor);
}

void FMFloatingPreview::mouseReleaseEvent(QMouseEvent * e)
{
	if(hasMouseGrab)
		releaseMouse();
	refPoint = QPoint();
	QApplication::restoreOverrideCursor();
}

void FMFloatingPreview::mouseMoveEvent(QMouseEvent * e)
{
	if (!(e->buttons() & Qt::LeftButton))
		return;
	QRect r(geometry());
	QPoint delta( e->globalPos() - refPoint);
	r.moveTo(delta);
	setGeometry(r);
}

void FMFloatingPreview::enterEvent(QEvent * e)
{
	menuWidget->childrenVisible(true);
}

void FMFloatingPreview::leaveEvent(QEvent * e)
{
	if(hasMouseGrab)
		releaseMouse();
	menuWidget->childrenVisible(false);
	QApplication::restoreOverrideCursor();
}


bool FMFloatingPreview::canTransparent()
{
#if defined(Q_WS_X11)
	return QX11Info::isCompositingManagerRunning();
#endif
	return true;
}
