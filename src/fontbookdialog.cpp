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
#include "fontitem.h"
#include "typotek.h"
#include "fmpaths.h"

#include <QFileDialog>
#include <QDebug>
#include <QGraphicsScene>
#include <QDomNodeList>
#include <QMessageBox>

FontBookDialog::FontBookDialog ( QWidget *parent )
		: QDialog ( parent )
{
	setupUi ( this );
	isOk = false;
	m_isTemplate = false;
// 	loadTemplateButton->setVisible(false);
// 	templateLabel->setVisible(false);
	curTemplatePreview = 0;

// 	fillSizeList();
	fillFontsList();
// 	QString alorem ( "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nUt a sapien. Aliquam aliquet purus molestie dolor.\nInteger quis eros ut erat posuere dictum. Curabitur dignissim.\nInteger orci. Fusce vulputate lacus at ipsum. \nQuisque in libero nec mi laoreet volutpat." );
// 	QString loremBig ( "LOREM IPSUM DOLOR" );
// 	setSampleHeadline ( loremBig );
// 	setSampleText ( alorem );
	
// 	m_pageRect = QRectF(0,0,m_pageSize.width(), m_pageSize.height());
// 	preScene = new QGraphicsScene(m_pageRect);
// // 	preView->setScene(preScene);
// 	preView->setRenderHint ( QPainter::Antialiasing, true );
// 	preView->setBackgroundBrush(Qt::lightGray);
	
	templateScene = new QGraphicsScene();
	templateScene->setBackgroundBrush(Qt::lightGray);
	templatePreview->setScene(templateScene);
	
	fillTemplates();
	
// 	slotPreview();
	
	connect ( okButton,SIGNAL ( accepted () ),this,SLOT ( slotAccept() ) );
	connect ( okButton,SIGNAL ( rejected() ),this,SLOT ( slotCancel() ) );
	connect ( fileNameButton,SIGNAL ( released() ),this,SLOT ( slotFileDialog() ) );
// 	connect ( paperSizeCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPageSize ( int ) ) );
// 	connect(this,SIGNAL(updateView()),this,SLOT(slotPreview()));
	
	//all Update
	connect(fileNameEdit,SIGNAL(textChanged( const QString& )),this,SIGNAL(updateView()));
// 	QList<QSpinBox*> spinList;
// 	spinList << familySpinBox;
// 	spinList << styleSpinBox;
// 	spinList << sampleSpinBox;
// 	spinList << familyFontSizeSpin;
// 	spinList << styleFontSizeSpin;
// 	spinList << headlineFontSizeSpin;
// 	spinList << bodyFontSizeSpin;
// 	foreach(QSpinBox *sp, spinList)
// 	{
// 		connect(sp,SIGNAL(valueChanged ( int  )),this,SIGNAL(updateView()));
// 	}
// 	connect (sampleTextEdit,SIGNAL(textChanged()),SIGNAL(updateView()));
// 	connect (sampleHeadline,SIGNAL(textChanged( const QString&)),SIGNAL(updateView()));
	
// 	connect(loadTemplateButton,SIGNAL(released()),this,SLOT(slotLoadTemplate()));
	
	connect( templatesList,SIGNAL(currentTextChanged( const QString& )),this,SLOT(slotPreviewTemplate(const QString&)));

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
	QString theFile = QFileDialog::getSaveFileName ( this, tr("Save fontBook"), QDir::homePath() , "Portable Document Format (*.pdf)" );
	fileNameEdit->setText ( theFile );
}

void FontBookDialog::fillFontsList()
{
	QList<FontItem*>localFontMap = typotek::getInstance()->getCurrentFonts();
	foreach(FontItem* fit, localFontMap)
	{
		selectedFontsList->addItem(fit->fancyName());
	}
}

QString FontBookDialog::getFileName()
{
	return fileNameEdit->text();
}

/**
*	1 - browse to select a template file
*	2 - load as a QDomDocument 
*	3 - search for "description" and "preview" elements
*	4 - check validity of the doc (will be hard at the beginning)
*/
void FontBookDialog::slotLoadTemplate(const QString &theTemplate)
{
// 	QString theTemplate = QFileDialog::getOpenFileName ( this, "Get template", QDir::homePath(), tr("Templates (*.xml)"));
	qDebug() << "FontBookDialog::slotLoadTemplate("<<theTemplate<<") -> " << templatesMap[theTemplate];
	if(theTemplate.isEmpty())
		return;
	
	QFile file(templatesMap[theTemplate]);
	QDomDocument doc("template");
	if ( !file.open ( QFile::ReadOnly ) )
	{
		QMessageBox::warning (0, QString ( "Fontmatrix" ),
				      QString ( "Can’t read %1." ).arg(file.fileName()) );
		return;
	}
	if ( !doc.setContent ( &file ) )
	{
		file.close();
		QMessageBox::warning (0, QString ( "Fontmatrix" ),
				      QString ( "%1 is an invalid XML tree." ).arg(file.fileName()) );
		return;
	}
	file.close();
	
	m_template = doc;
	m_isTemplate = true;
}

void FontBookDialog::fillTemplates()
{
	
	QDir tDir(typotek::getInstance()->getTemplatesDir());
	QStringList filters;
	filters << "*.xml" ;
	tDir.setNameFilters ( filters );
	QStringList pathList = tDir.entryList();
	for ( int i = 0 ; i < pathList.count() ; ++i )
	{
		QFile file(tDir.absoluteFilePath ( pathList.at ( i ) ));
		QDomDocument doc("template");
		if ( !file.open ( QFile::ReadOnly ) )
		{
			QMessageBox::warning (0, QString ( "Fontmatrix" ),
					      QString ( "Can’t read %1." ).arg(file.fileName()) );
			return;
		}
		if ( !doc.setContent ( &file ) )
		{
			file.close();
			QMessageBox::warning (0, QString ( "Fontmatrix" ),
					      QString ( "%1 is an invalid XML tree." ).arg(file.fileName()) );
			return;
		}
		file.close();
	
		QString description;
		QDomNodeList descList = doc.elementsByTagName ( "name" );
		if ( descList.length()  )
		{
			QDomNode node = descList.item ( 0 );
			description = node.toElement().text();
		}
		QString preview;
		QDomNodeList prevList = doc.elementsByTagName ( "preview" );
		if ( descList.length()  )
		{
			QDomNode node = prevList.item ( 0 );
			preview = tDir.absoluteFilePath( node.toElement().text() );
		}
		
		if(description.isEmpty())
			continue;
		
		templatesMap[description] = tDir.absoluteFilePath ( pathList.at ( i ) );
		
		if(!preview.isEmpty())
		{
			templatesPreviewMap[description] = QPixmap(preview);
		}
		
		
	}
	// Here we insert default templates provided by Fontmatrix
	templatesMap["Default template"] = FMPaths::ResourcesDir() + "template_default";
	templatesPreviewMap["Default template"] = QPixmap(FMPaths::ResourcesDir() +"template_default_preview");
	templatesMap["Default template (oneliner)"] = FMPaths::ResourcesDir() +"template_oneline";
	templatesPreviewMap["Default template (oneliner)"] =  QPixmap(FMPaths::ResourcesDir() +"template_oneline_preview");
	
	templatesList->addItems(templatesMap.keys());	
}

void FontBookDialog::slotPreviewTemplate(const QString &key)
{
	qDebug() << "slotPreviewTemplate("<<key<<") -> "<< templatesMap[key];
	if(templatesMap.contains(key))
	{
		if(!templatesPreviewMap[key].isNull())
		{
			if(curTemplatePreview)
				templateScene->removeItem(curTemplatePreview);
			delete curTemplatePreview;
			
			curTemplatePreview = templateScene->addPixmap( templatesPreviewMap[key] );
// 			templatePreview->ensureVisible(curTemplatePreview,10,10);
		}
		
		slotLoadTemplate(key);
		
	}
	
}
