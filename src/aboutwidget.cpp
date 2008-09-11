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
#include "aboutwidget.h"
#include "typotek.h"
#include "fmfontdb.h"

AboutWidget::AboutWidget(QWidget *parent)
	:QDialog(parent)
{
	setupUi ( this );
// 	theText->setOpenExternalLinks ( true );
	QString version_maj(QString::number( FONTMATRIX_VERSION_MAJOR) );
	QString version_min(QString::number( FONTMATRIX_VERSION_MINOR) );
	QString version_pat(QString::number( FONTMATRIX_VERSION_PATCH) );
	theText->setSource(QUrl("qrc:/texts/about"));
	theText_2->setSource(QUrl("qrc:/texts/about_people"));
	versionStringLabel->setText(tr("version ") + version_maj + "." + version_min + "." + version_pat);
	
	fontsCountLabel->setText(QString::number(FMFontDb::DB()->FontCount()) + " " +tr("fonts loaded") );
	
			
}


AboutWidget::~AboutWidget()
{
}


