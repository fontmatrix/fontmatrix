//
// C++ Interface: prefspaneldialog
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PREFSPANELDIALOG_H
#define PREFSPANELDIALOG_H

#include <qdialog.h>
#include <ui_prefs_panel.h>


/**
	@author Pierre Marchand <pierremarc@oep-h.com>
*/
class PrefsPanelDialog : public QDialog, private Ui::PrefsPanel
{
	Q_OBJECT
	public:
		PrefsPanelDialog ( QWidget *parent );

		~PrefsPanelDialog();

	private:
		void doConnect();
	private slots:
		void applySampleText();

};

#endif
