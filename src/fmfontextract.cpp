//
// C++ Implementation: fmfontextract
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmfontextract.h"

#include <QFileInfo>
#include <QFileDialog>

#ifdef HAVE_PODOFO
#include "fmpdffontextractor.h"
#endif

FMFontExtract::FMFontExtract(QWidget * parent)
	:QDialog(parent), lastPath(QDir::homePath()), lastDir(QDir::homePath())
{
	setupUi(this);
#ifdef HAVE_PODOFO	
	extractorPDF = 0;
#endif
	docPath->clear();
	
	connect(browsePDF,SIGNAL(clicked()),this,SLOT(slotBrowseDoc()));
	connect(browseDir,SIGNAL(clicked()),this,SLOT(slotBrowseDir()));
	connect(extractButton,SIGNAL(clicked()),this,SLOT(slotExtract()));
	
}

FMFontExtract::~ FMFontExtract()
{
#ifdef HAVE_PODOFO
	if(extractorPDF)
		delete extractorPDF;
#endif
}

void FMFontExtract::loadDoc(const QString & path)
{
	QFileInfo fi(path);
	if(!fi.exists())
	{
		docPath->setText(tr("File does not exist:") + " " + fi.fileName());
		return;
	}
	if((fi.suffix() == "pdf") || (fi.suffix() == "PDF"))
	{
#ifdef HAVE_PODOFO
		if(extractorPDF)
			delete extractorPDF;
		fontList->clear();
		
		extractorPDF = new FMPDFFontExtractor(path);
		foreach(QString n, extractorPDF->list())
		{
			fontList->addItem(n);
		}
#else
		docPath->setText(tr("Not built with PDF extractor."));
#endif
	}
	else
	{
		docPath->setText(tr("Format not handled."));
	}
		
	for(int i(0);i < fontList->count(); ++i )
	{
		fontList->item(i)->setCheckState(Qt::Unchecked);
	}
}

void FMFontExtract::slotBrowseDoc()
{
	QString filters( "Portable Document Format (*.pdf *.PDF)" );
	QString path( QFileDialog::getOpenFileName ( this, "Fontmatrix", lastPath ,filters) );
	if(path.isEmpty())
		return;
	lastPath = path;
	docPath->setText(path);
	loadDoc(path);
}

void FMFontExtract::slotBrowseDir()
{
	QString dirpath(QFileDialog::getExistingDirectory(this,"Fontmatrix",lastDir));
	if(dirpath.isEmpty())
		return;
	lastDir = dirpath;
	outputDir->setText(dirpath);
}

void FMFontExtract::slotExtract()
{
	if(fontList->count() == 0)
		return;
	
	QStringList names;
	for(int i(0);i < fontList->count(); ++i )
	{
		if(fontList->item(i)->checkState() == Qt::Checked)
			names << fontList->item(i)->text();
	}
	QString odir(outputDir->text() + QDir::separator());

	foreach(QString name,names)
	{
		QString ext = ".xxx";
#ifdef HAVE_PODOFO
		ext = "." + extractorPDF->fontType(name);
#endif	
	
		QString fnam(odir + name + ext);
		if(QFile::exists(fnam))
			QFile::remove(fnam);
		QFile f(fnam);
		if(f.open(QIODevice::WriteOnly))
		{
#ifdef HAVE_PODOFO
			if(extractorPDF)
				extractorPDF->write(name, &f);
#endif
		}
	}
	
}
