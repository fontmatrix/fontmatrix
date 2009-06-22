//
// C++ Interface: fmnamelist
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMNAMELIST_H
#define FMNAMELIST_H

#include <QTime>
#include <QTreeWidget>
#include <QString>

class QTreeViewItem;

class FMNameList : public QTreeWidget
{
	Q_OBJECT
	public:
		FMNameList(QWidget *parent);
		~FMNameList();
	public slots:
		void slotNextFamily();
		void slotPreviousFamily();
		void slotNextFont();
		void slotPreviousFont();
		bool slotSetCurrent(const QString& fname);
		
	signals:
		void currentChanged(QTreeWidgetItem*, int);
	protected:
		void keyPressEvent ( QKeyEvent * e );
// 	private:
// 		QString curString;
	private:
		bool findAbove(QTreeWidgetItem *current, const QString &role);
		bool findBelow(QTreeWidgetItem *current, const QString &role);
		
		QString m_keyString;
		QTime m_keyTime;
		const int m_waitKey;
};

#endif

