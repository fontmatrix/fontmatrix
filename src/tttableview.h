//
// C++ Interface: tttableview
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TTTABLEVIEW_H
#define TTTABLEVIEW_H

#include "ui_tttablewidget.h"

class FontItem;
class QTreeWidgetItem;

class TTTableView : public QWidget, private Ui::TTTableWidget
{
	enum Tfield_p{
		NAME = 0,
		DESCRIPTION = 1,
		SIZE = 2	
	};
	
	QList<QTreeWidgetItem*> twiList;
	
	public:
		
		TTTableView(FontItem * font, QWidget * parent = 0);
		~TTTableView();
		
};

#endif
