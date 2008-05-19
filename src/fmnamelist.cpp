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
	if (!findBelow(currentItem(), "family")) {
		/* Goto first family on the list */
		findBelow(topLevelItem(0), "family");
	}
}

void FMNameList::slotPreviousFamily()
{
	if (!findAbove(currentItem(), "family")) {
		/*TODO Goto last family on the list */
	}
}

void FMNameList::slotNextFont()
{
	if (!findBelow(currentItem(), "fontfile")) {
		/*Goto first font on the list */
		findBelow(topLevelItem(0), "fontfile");
	}
}

void FMNameList::slotPreviousFont()
{
	if (!findAbove(currentItem(), "fontfile")) {
		/*TODO Goto last font on the list! */
	}
}

bool FMNameList::findAbove(QTreeWidgetItem *current, const QString &role)
{
	QTreeWidgetItem *above = current;
	if (current) {
		setCurrentItem(current);
		above = itemAbove(current);
		if (!above)
			return false;

		if (!above->isExpanded())
			above->setExpanded(true);

		while (above->data ( 0,100 ).toString() != role)
		{
			above = itemAbove(above);
			if (!above)
				break;
			else {
				if (!above->isExpanded())
					above->setExpanded(true);
			}
		}
	}
	if (above) {
		setCurrentItem(above);
		emit currentChanged(above, 0);
		return true;
	} else
		return false;
}

bool FMNameList::findBelow(QTreeWidgetItem *current, const QString &role)
{
	QTreeWidgetItem *below = current;
	if (current) {
		setCurrentItem(current);
		below = itemBelow(current);
		if (!below)
			return false;

		if (!below->isExpanded())
			below->setExpanded(true);

		while (below->data ( 0,100 ).toString() != role)
		{
			below = itemBelow(below);
			if (!below)
				break;
			else {
				if (!below->isExpanded())
					below->setExpanded(true);
			}
		}
	}
	if (below) {
		setCurrentItem(below);
		emit currentChanged(below, 0);
		return true;
	} else
		return false;
}
