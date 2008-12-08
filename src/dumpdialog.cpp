//
// C++ Implementation: dumpdialog
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "dumpdialog.h"
#include "fmdumpinfo.h"
#include "fontitem.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>

FMDumpDialog::FMDumpDialog(FontItem * font, QWidget * parent)
	: QDialog(parent), m_dumpinfo(0)
{
	setupUi(this);
	fontName->setText(font->fancyName());
	m_dumpinfo = new FMDumpInfo(font);
	nameList->addItems(m_dumpinfo->infos());
	
	connect(browseButton, SIGNAL(clicked()), this, SLOT(browseFile()));
	connect(loadButton, SIGNAL(clicked()), this, SLOT(browseModel()));
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotDumpIt()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	connect(modelText, SIGNAL(insertContent()), this, SLOT(insertSelectedField()));
}

FMDumpDialog::~ FMDumpDialog()
{
	if(m_dumpinfo)
		delete m_dumpinfo;
}

QString FMDumpDialog::getModel() const
{
	return modelText->toPlainText();
}

QString FMDumpDialog::getFilePath() const
{
	return filePath->text();
}

void FMDumpDialog::slotDumpIt()
{
	m_dumpinfo->setModel(getModel());
	
	if(m_dumpinfo->dumpInfo(getFilePath()))
		accept();
	else
		reject();
}

void FMDumpDialog::browseFile()
{
	QString s( QFileDialog::getSaveFileName(this, "Fontmatrix", QDir::homePath()) );
	if(!s.isEmpty())
	{
		filePath->setText(s);
	}
}

void FMDumpDialog::browseModel()
{
	QString s( QFileDialog::getOpenFileName(this, "Fontmatrix", QDir::homePath()) );
	if(!s.isEmpty())
	{
		QFile file(s);
		if(file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QTextStream ts(&file);
			modelText->setPlainText( ts.readAll() );
			
		}
		file.close();
	}
	else
		modelText->setPlainText( "" );
}

void FMDumpDialog::insertSelectedField()
{
	if(!nameList->selectedItems().isEmpty())
	{
		QTextCursor cursor = modelText->textCursor();
		cursor.insertText(m_dumpinfo->info( nameList->selectedItems().first()->text()));
	}
}
