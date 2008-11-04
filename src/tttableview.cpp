//
// C++ Implementation: tttableview
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QDebug>

#include "tttableview.h"

#include "fontitem.h"
#include "fmfontstrings.h"

TTTableView::TTTableView(FontItem * font, QWidget * parent)
	:QWidget(parent)
{
	setupUi(this);
// 	tView->setColumnCount (3);
	foreach( QString tname, FontStrings::Tables().keys() )
	{
		int len(font->table(tname));
		if(len > 0)
		{
			QTreeWidgetItem *twi(new QTreeWidgetItem);
			twi->setText(NAME, tname);
			twi->setText(DESCRIPTION, FontStrings::Tables()[tname]);
			twi->setText(SIZE, QString::number(len));
			twiList << twi;
			tView->addTopLevelItem(twi);
		}
// 		else
// 			qDebug()<<tname<<len;
	}
	tView->resizeColumnToContents(DESCRIPTION);
}

TTTableView::~ TTTableView()
{
	foreach(QTreeWidgetItem *twi, twiList)
	{
		delete twi;
	}
}
