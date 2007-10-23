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
#include "fontbookdialog.h"
#include <QFileDialog>
#include <QDebug>


FontBookDialog::FontBookDialog ( QWidget *parent )
		: QDialog ( parent )
{
	setupUi ( this );
	isOk = false;

	fillSizeList();
	QString alorem ( "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nUt a sapien. Aliquam aliquet purus molestie dolor.\nInteger quis eros ut erat posuere dictum. Curabitur dignissim.\nInteger orci. Fusce vulputate lacus at ipsum. \nQuisque in libero nec mi laoreet volutpat." );
	QString loremBig ( "LOREM IPSUM DOLOR" );
	setSampleHeadline ( loremBig );
	setSampleText ( alorem );
	connect ( okButton,SIGNAL ( accepted () ),this,SLOT ( slotAccept() ) );
	connect ( okButton,SIGNAL ( rejected() ),this,SLOT ( slotCancel() ) );
	connect ( fileNameButton,SIGNAL ( released() ),this,SLOT ( slotFileDialog() ) );
	connect ( paperSizeCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPageSize ( int ) ) );
}


FontBookDialog::~FontBookDialog()
{
}

void FontBookDialog::slotAccept()
{
	isOk = true;
	close();
}

void FontBookDialog::slotCancel()
{
	isOk = false;
	close();
}

void FontBookDialog::slotFileDialog()
{
	QString theFile = QFileDialog::getSaveFileName ( this, "Save fontBook", QDir::homePath() , "Portable Document Format (*.pdf)" );
	fileNameEdit->setText ( theFile );
}

double FontBookDialog::getTabFamily()
{
	return familySpinBox->value();
}

double FontBookDialog::getTabStyle()
{
	return styleSpinBox->value();
}

double FontBookDialog::getTabSampleText()
{
	return sampleSpinBox->value();
}

void FontBookDialog::setSampleText ( QString s )
{
	sampleTextEdit->setPlainText ( s );
}

void FontBookDialog::setSampleHeadline ( QString s )
{
	sampleHeadline->setText ( s );
}

QString FontBookDialog::getSampleHeadline()
{
	return sampleHeadline->text();
}


QString FontBookDialog::getSampleText()
{
	return sampleTextEdit->toPlainText();
}

QString FontBookDialog::getFileName()
{
	return fileNameEdit->text();
}

QSizeF FontBookDialog::getPageSize()
{
	return m_pageSize;

}

void FontBookDialog::fillSizeList()
{
	paperSizeCombo->addItem ( "A0",QSizeF (33.1 * 72.0, 46.8 * 72.0 ) );
	paperSizeCombo->addItem ( "A1",QSizeF (23.4 * 72.0, 33.1 * 72.0 ) );
	paperSizeCombo->addItem ( "A2",QSizeF (16.5 * 72.0, 23.4 * 72.0 ) );
	paperSizeCombo->addItem ( "A3",QSizeF (11.7 * 72.0, 16.5 * 72.0 ) );
	paperSizeCombo->addItem ( "A4",QSizeF ( 8.3 * 72.0, 11.7 * 72.0 ) );
	paperSizeCombo->addItem ( "A5",QSizeF ( 5.8 * 72.0,  8.3 * 72.0 ) );
	// default to A4
	m_pageSize = QSizeF (	8.3  * 72.0, 11.7 * 72.0 );
	m_pageSizeConstant = QPrinter::A4;
	paperSizeCombo->setCurrentIndex ( 4 );

}

double FontBookDialog::getFontSize ( QString s )
{
	if ( s == "family" )
	{
		return familyFontSizeSpin->value();
	}
	if ( s == "style" )
	{
		return styleFontSizeSpin->value();
	}
	if ( s == "headline" )
	{
		return headlineFontSizeSpin->value();
	}
	if ( s == "body" )
	{
		return bodyFontSizeSpin->value();
	}
	return 0.0;
}

void FontBookDialog::slotPageSize ( int index )
{
	QVariant var= paperSizeCombo->itemData ( index );
	if ( var.isValid() )
	{
		m_pageSize = var.toSizeF();
		if(paperSizeCombo->itemText(index)  == "A0")
		{
			m_pageSizeConstant = QPrinter::A0;
		}
		if(paperSizeCombo->itemText(index)  == "A1")
		{
			m_pageSizeConstant = QPrinter::A1;
		}
		if(paperSizeCombo->itemText(index)  == "A2")
		{
			m_pageSizeConstant = QPrinter::A2;
		}
		if(paperSizeCombo->itemText(index)  == "A3")
		{
			m_pageSizeConstant = QPrinter::A3;
		}
		if(paperSizeCombo->itemText(index)  == "A4")
		{
			m_pageSizeConstant = QPrinter::A4;
		}
		if(paperSizeCombo->itemText(index)  == "A5")
		{
			m_pageSizeConstant = QPrinter::A5;
		}
	}
	else
	{
		qDebug() << "invalid QVARIANT";
	}
}

QPrinter::PageSize FontBookDialog::getPageSizeConstant()
{
	return m_pageSizeConstant;
}





