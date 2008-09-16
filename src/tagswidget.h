//
// C++ Interface: tagswidget
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include "ui_tagswidget.h"

class FontItem;

class TagsWidget : public QWidget , private Ui::tagsWidget
{
	Q_OBJECT
		TagsWidget(QWidget * parent);
		~TagsWidget();
		static TagsWidget * instance;
		
		QList<FontItem*> theTaggedFonts;
		
	public:
		static TagsWidget *getInstance();
		void prepare(QList<FontItem*> fonts);
		void newTag();
		void removeFromTagged(FontItem* f){theTaggedFonts.removeAll(f);}
		
	private slots:
		void slotSwitchCheckState( QListWidgetItem * item );
		void slotNewTag();
		void slotFinalize();
};

#endif //TAGSWIDGET_H

