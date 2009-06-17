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
#include <QMessageBox>

#ifdef HAVE_PODOFO
#include "fmpdffontextractor.h"
#endif

FMFontExtract::FMFontExtract(QWidget * parent)
	:QDialog(parent), lastPath(QDir::homePath()), lastDir(QDir::homePath())
{
	setupUi(this);
	currentExtractor = 0;
#ifdef HAVE_PODOFO
	FMPDFFontExtractor * pdfExtr(new FMPDFFontExtractor);
	foreach(QString e, pdfExtr->extensions())
	{
		extractors[e] = pdfExtr;
	}
#endif
	
	
	
	docPath->clear();
	
	connect(browsePDF,SIGNAL(clicked()),this,SLOT(slotBrowseDoc()));
	connect(browseDir,SIGNAL(clicked()),this,SLOT(slotBrowseDir()));
	connect(extractButton,SIGNAL(clicked()),this,SLOT(slotExtract()));
	
}

FMFontExtract::~ FMFontExtract()
{
	QList<FMFontExtractorBase*> extP;
	foreach(FMFontExtractorBase* b, extractors.values())
	{
		if(!extP.contains(b))
			extP << b;
	}
	foreach(FMFontExtractorBase* b, extP)
	{
		if(b)
			delete b;
	}
}

void FMFontExtract::loadDoc(const QString & path)
{
	QFileInfo fi(path);
	if(!fi.exists())
	{
		docPath->setText(tr("File does not exist:") + " " + fi.fileName());
		return;
	}
	
	QString suffix(fi.suffix());

	if(extractors.contains(suffix))
	{
		currentExtractor = extractors[suffix];
		fontList->clear();
		if(currentExtractor->loadFile(path))
		{
			foreach(QString n, currentExtractor->list())
			{
				fontList->addItem(n);
			}
		}
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
// 	QString filters( "Portable Document Format (*.pdf *.PDF)" );
	QString path( QFileDialog::getOpenFileName ( this, "Fontmatrix", lastPath) );
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

	QStringList failedExt;
	foreach(QString name,names)
	{
	
		QString fnam(odir + name + "." + currentExtractor->fontType(name));
		if(QFile::exists(fnam))
			QFile::remove(fnam);
		QFile f(fnam);
		if(f.open(QIODevice::WriteOnly))
		{
			if(!currentExtractor->write(name, &f))
				failedExt << name;
		}
	}
	if(!failedExt.isEmpty())
	{
		QMessageBox::information(this,"Fontmatrix",tr("Failed to extract:\n%1").arg(failedExt.join("\n")));
	}
	
}
