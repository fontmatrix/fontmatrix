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
	:QWidget(parent), m_font(font)
{
	setupUi(this);
// 	tView->setColumnCount (3);
	bool hasTable(false);
	QTreeWidgetItem *first;
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
			if(!hasTable)
			{
				hasTable = true;
				first = twi;
			}
		}
// 		else
// 			qDebug()<<tname<<len;
	}
	tView->resizeColumnToContents(DESCRIPTION);
	
	connect(tView,SIGNAL(itemSelectionChanged()),this,SLOT(updateHexView()));
	
	if(hasTable)
	{
		tView->setItemSelected(first,true);
		updateHexView();
	}
	
}

TTTableView::~ TTTableView()
{
	foreach(QTreeWidgetItem *twi, twiList)
	{
		delete twi;
	}
}

void TTTableView::updateHexView()
{
	if(!tView->selectedItems().count())
		return;
	
	QString table(tView->selectedItems()[0]->text(NAME));
	QByteArray ar(m_font->tableData(table));
	
	m_data.clear();
	for(int i(0);i<ar.count();++i)
	{
		m_data << ar.at(i);
	}
	hexView->setData(&m_data);
}
