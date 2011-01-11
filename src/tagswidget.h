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
class TagsWidget_ListModel;

class TagsWidget : public QWidget , private Ui::tagsWidget
{
	Q_OBJECT


	TagsWidget_ListModel * model;

public:
	TagsWidget(QWidget * parent);
	~TagsWidget();

	void prepare(QList<FontItem*> fonts);

private slots:
	void slotNewTag();
	void slotActRemovetag();

};

#endif //TAGSWIDGET_H

