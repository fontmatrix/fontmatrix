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
#include <QFileDialog>
#include <QFile>

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
	connect(exportButton,SIGNAL(clicked()),this,SLOT(exportHex()));
	
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
	curTable = m_font->tableData(table);
	
	m_data.clear();
	for(int i(0);i<curTable.count();++i)
	{
		m_data << curTable.at(i);
	}
	hexView->setData(&m_data);
}

void TTTableView::exportHex()
{
	if(curTable.isEmpty())
		return;
	
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
	if(fileName.isEmpty())
		return;
	QFile f(fileName);
	if(f.open(QIODevice::WriteOnly))
	{
		f.seek(0);
		
		f.write(curTable);
	}
}


