//
// C++ Interface: dumpdialog
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef DUMPDIALOG_H
#define DUMPDIALOG_H

#include "ui_dumpdialog.h"

class FontItem;
class FMDumpInfo;

class FMDumpDialog : public QDialog, private Ui::DumpDialog
{
	Q_OBJECT
	public:
		FMDumpDialog(FontItem * font, QWidget * parent);
		~FMDumpDialog();

		QString getModel() const;
		QString getFilePath() const;
		
	private:
		FMDumpInfo * m_dumpinfo;
		
	private slots:
		void slotDumpIt();
		void browseFile();
		void browseModel();
		
		void insertSelectedField();
};

#endif // DUMPDIALOG_H
