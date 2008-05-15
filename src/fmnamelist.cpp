//
// C++ Implementation: fmnamelist
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmnamelist.h"

#include <QKeyEvent>
#include <QTreeWidgetItem>
#include <QDebug>

FMNameList::FMNameList(QWidget * parent)
	:QTreeWidget(parent)
{
}

FMNameList::~ FMNameList()
{
}

void FMNameList::keyPressEvent(QKeyEvent * e)
{
// 	qDebug()<<"FMNameList::keyPressEvent("<<e<<")";
	if(e->text().isEmpty())
		return;


	QTreeWidgetItem *item = 0;
	QString alpha = e->text().toUpper();
// 	if(e->modifiers().testFlag( Qt::ControlModifier ))
// 	{
// 		curString += e->text();
// 		alpha = curString;
// 	}
// 	else
// 	{
// 		curString = "";
// 	}
	for(int i(0); i < topLevelItemCount() ; ++i)
	{
		item = topLevelItem(i);
		if(item->data(0,100).toString() == "alpha" && item->text(0) == alpha)
		{
			scrollToItem(item, QAbstractItemView::PositionAtTop);
			return;
		}
	}
// 	QTreeWidget::keyPressEvent(e);
}

void FMNameList::slotNextFamily()
{
	QTreeWidgetItem *current = currentItem();
	if (current) {
		qDebug("current: %d", topLevelItemCount());
	} else {
		qDebug("no current: %d", topLevelItemCount());
	}
}

void FMNameList::slotPreviousFamily()
{

}

void FMNameList::slotNextFont()
{
	QTreeWidgetItem *current = currentItem();
	if (current) {
		QTreeWidgetItem *below = itemBelow(current);
		setCurrentItem(below);
		emit currentChanged(below, 0);
	} else {

	}
}

void FMNameList::slotPreviousFont()
{
	QTreeWidgetItem *current = currentItem();
	if (current) {
		QTreeWidgetItem *above = itemAbove(current);
		setCurrentItem(above);
		emit currentChanged(above, 0);
	} else {

	}
}
