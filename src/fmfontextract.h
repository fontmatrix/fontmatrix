//
// C++ Interface: fmfontextract
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMFONTEXTRACT_H
#define FMFONTEXTRACT_H

#include "ui_fontextractordialog.h"

#ifdef HAVE_PODOFO
class FMPDFFontExtractor;
#endif

class FMFontExtract : public QDialog, private Ui::FontExtractorDialog
{
		Q_OBJECT

	public:
		FMFontExtract ( QWidget * parent );
		~FMFontExtract();

	private:
#ifdef HAVE_PODOFO
		FMPDFFontExtractor * extractorPDF;
#endif
		void loadDoc(const QString& path);
		QString lastPath;
		QString lastDir;
		
	private slots:
		void slotBrowseDoc();
		void slotBrowseDir();
		void slotExtract();
};

#endif // FMFONTEXTRACT_H
