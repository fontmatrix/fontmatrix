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

	static FontCompareWidget* instance;
	FontCompareWidget(QWidget * parent);

	public:
		static FontCompareWidget* getInstance();
		~FontCompareWidget();
		
	private:
		QString curFont;
		uint curcode;
		bool neverUsed;
		
		void doconnect();
		void dodisconnect();
		void resetElements();
		
		void initColors();
		
	private slots:
		void addFont();
		void removeFont();
		void showChange();
		void fillChange(int newIdx);
		void pointsChange();
		void controlsChange();
		void metricsChange();
		void offsetChange(int o);
		void characterChange(int v);
		void characterBoxChange(int i);
		void fontChange(QListWidgetItem * witem, QListWidgetItem * olditem = 0);
		void syncChange(int state);
		
};

#endif
