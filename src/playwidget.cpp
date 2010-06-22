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

#include "playwidget.h"
#include "ui_playwidget.h"

PlayWidget* PlayWidget::instance = 0;
PlayWidget::PlayWidget() :
    ui(new Ui::PlayWidget)
{
    ui->setupUi(this);
    playScene = new QGraphicsScene;
    playScene->setSceneRect ( 0,0,10000,10000 );
    ui->playView->setScene( playScene );

    connect ( ui->playView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
}

PlayWidget::~PlayWidget()
{
    delete ui;
}

PlayWidget* PlayWidget::getInstance()
{
	if(instance == 0)
	{
		instance = new PlayWidget;
		Q_ASSERT(instance);
	}
	return instance;
}

void PlayWidget::changeEvent(QEvent *e)
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
void PlayWidget::slotZoom ( int z )
{
	double delta =  1.0 + ( z/1000.0 ) ;
	QTransform trans;
	trans.scale ( delta,delta );
	ui->playView->setTransform ( trans, ( z == 0 ) ? false : true );
}

double PlayWidget::playFontSize()
{
	return ui->playFontSize->value();
}

QRectF PlayWidget::getMaxRect()
{
	return ui->playView->getMaxRect();
}

void PlayWidget::clearSelection()
{
	ui->playView->deselectAll();
}
