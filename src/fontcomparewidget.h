//
// C++ Interface: fontcomparewidget
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FONTCOMPAREWIDGET_H
#define FONTCOMPAREWIDGET_H


#include "ui_comparewidget.h"

class FontCompareWidget : public QWidget, private Ui::CompareWidget
{
	Q_OBJECT
	public:
		FontCompareWidget(QWidget * parent);
		~FontCompareWidget();
		
	private:
		QString curFont;
		
	private slots:
		void addFont();
		void removeFont();
		void fillChange();
		void pointsChange();
		void controlsChange();
		void metricsChange();
		void characterChange(int v);	
		void fontChange(QListWidgetItem * witem, QListWidgetItem * olditem);
};

#endif
