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
	:QTreeWidget(parent) , m_waitKey (2000 /*ms*/)
{
	
}

FMNameList::~ FMNameList()
{
}

void FMNameList::keyPressEvent(QKeyEvent * e)
{
	if(e->text().isEmpty())
		return;
	int t(m_keyTime.elapsed());
// 	qDebug()<<"FMNameList::keyPressEvent"<<e<<m_keyString<<QString("%1").arg(t);

	if(m_keyString.isEmpty() 
		  || (t > m_waitKey) )
	{
		m_keyString = e->text().toUpper();
		QTreeWidgetItem *item = 0;
		for(int i(0); i < topLevelItemCount() ; ++i)
		{
			item = topLevelItem(i);
			if(item->data(0,100).toString() == "alpha" && item->text(0) == m_keyString)
			{
				scrollToItem(item, QAbstractItemView::PositionAtTop);
				m_keyTime.start();
				return;
			}
		}
		
	}
	else if(t < m_waitKey)
	{
		m_keyString += e->text().toUpper();
				
		int tli(topLevelItemCount());
		for(int i(0); i < tli ; ++i)
		{
			QTreeWidgetItem * TL(topLevelItem(i));
			for(int family(0); family < TL->childCount(); ++family)
			{
				if( TL->child(family)->text(0).toUpper().startsWith(m_keyString) )
				{
					scrollToItem(TL->child(family) , QAbstractItemView::PositionAtTop);
					m_keyTime.start();
					return;
				}
			}
		}
	}
	else
	{
		m_keyString.clear();
	}


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

void FMNameList::slotSetCurrent(const QString & fname)
{
// 	qDebug()<<"FMNameList::slotSetCurrent"<<fname;
	int tli(topLevelItemCount());
	for(int i(0); i < tli ; ++i)
	{
		QTreeWidgetItem * TL(topLevelItem(i));
		for(int family(0); family < TL->childCount(); ++family)
		{
			for(int face(0); face < TL->child(family)->childCount(); ++face)
			{
				QTreeWidgetItem * F( TL->child(family)->child(face) );
				if(F->toolTip(0) == fname)
				{
					setCurrentItem(F);
					emit currentChanged(F,0);
					return;
				}
			}
		}
	}
}

