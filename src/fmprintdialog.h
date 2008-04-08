//
// C++ Interface: fmprintdialog
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMPRINTDIALOG_H
#define FMPRINTDIALOG_H

#include "ui_multiprintdialog.h"

class QPrinter;

class FMPrintDialog : public QDialog, private Ui::multiPrintDialog
{
	Q_OBJECT
	public:
		FMPrintDialog(QPrinter * printer, QWidget * parent);
		~FMPrintDialog();
		
};


#endif
