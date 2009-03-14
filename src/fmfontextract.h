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

#include <QMap>

#include "ui_fontextractordialog.h"
#include "fmfontextractorbase.h"

class FMFontExtract : public QDialog, private Ui::FontExtractorDialog
{
		Q_OBJECT

	public:
		FMFontExtract ( QWidget * parent );
		~FMFontExtract();

	private:
		QMap<QString,FMFontExtractorBase*> extractors;
		FMFontExtractorBase* currentExtractor;
				
		void loadDoc(const QString& path);
		QString lastPath;
		QString lastDir;
		
	private slots:
		void slotBrowseDoc();
		void slotBrowseDir();
		void slotExtract();
};

#endif // FMFONTEXTRACT_H
