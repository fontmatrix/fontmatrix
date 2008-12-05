//
// C++ Interface: panosedialog
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PANOSEDIALOG_H
#define PANOSEDIALOG_H

#include "ui_panosedialog.h"

class FontItem;
class QComboBox;
class QLabel;

class FMPanoseDialog : public QDialog, private Ui::PanoseDialog
{
	Q_OBJECT
	public:
		FMPanoseDialog(FontItem * font, QWidget *parent);
		~FMPanoseDialog();
		
		QString getSourcePanose() const{return m_sourcepanose;}
		QString getTargetPanose() const{return m_targetpanose;}
		bool getOk() const{return m_ok;}
	private:
		FontItem *m_font;
		QString m_sourcepanose;
		QString m_targetpanose;
		QMap<QString, QComboBox*> m_box;
		QMap<QString, QLabel*> m_label;
		bool m_ok;
		
		void populateDialog();
		
	private slots:
		void panoseChange( int index );
		void closeOk();
		void closeCancel();
};

#endif //PANOSEDIALOG_H
