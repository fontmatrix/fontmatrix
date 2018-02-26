/***************************************************************************
 *   Copyright (C) 2009 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef PANOSEWIDGET_H
#define PANOSEWIDGET_H

#include <QWidget>
#include <QItemSelection>

class QTreeWidgetItem;

namespace Ui {
	class PanoseWidget;
}


class PanoseWidget : public QWidget {
	Q_OBJECT

public:
	PanoseWidget(QWidget *parent = 0);
	~PanoseWidget();

	void setFilter(const QMap<int, QList<int> >& filter);
	QMap<int, QList<int> > getFilter() const{return m_filter;}

protected:
	//    void changeEvent(QEvent *e);
	void closeEvent(QCloseEvent *);

private:
	Ui::PanoseWidget *m_ui;

	int m_filterKey;
	QMap<int, QList<int> > m_filter;

	void doConnect(const bool& c);

private slots:
	void slotSelect(QTreeWidgetItem * item, int column);

signals:
	void filterChanged();

};

#endif // PANOSEWIDGET_H
