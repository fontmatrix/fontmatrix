//
// C++ Interface: fmlayoptwidget
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMLAYOPTWIDGET_H
#define FMLAYOPTWIDGET_H

#include <ui_layoutoptions.h>

class FMLayOptWidget : public QWidget , private Ui::LayoutOptionWidget
{
	Q_OBJECT
	public:
		enum V{BEFORE = 1, EXACT, AFTER, END, HYPHEN, SPACE, MAX};
		
		FMLayOptWidget(QWidget * parent = 0);
		
		int vToInt(V v){return v;}
		void setRange(V v, int min, int max);
		void setValue(V v, int value);
		double getValue(V v);

	private slots:
		void bChanged(int cv);
		void exChanged(int cv);
		void aChanged(int cv);
		void enChanged(int cv);
		void hChanged(int cv);
		void sChanged(int cv);
		
		void bEdited();
		void exEdited();
		void aEdited();
		void enEdited();
		void hEdited();
		void sEdited();
		
	signals:
		/// Indicates which slider has been changed
		/// It’s up to the receiver to ask the new value;
		void valueChanged(int);
		
};

#endif

