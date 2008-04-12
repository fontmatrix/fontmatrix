//
// C++ Implementation: fmprintdialog
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//



#include "fmprintdialog.h"
#include <QPrintDialog>


FMPrintDialog::FMPrintDialog(QPrinter * printer, QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	QPrintDialog  printDialog(printer, printPlaceHolder);
	printDialog.setModal(false);
	printDialog.show();
// 	printDialog.exec();
}

FMPrintDialog::~ FMPrintDialog()
{
}

