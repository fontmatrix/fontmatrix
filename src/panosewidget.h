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

#include <QtGui/QWidget>
#include <QModelIndex>
#include <QItemSelection>

class PanoseValueModel;
class PanoseAttributeModel;

namespace Ui {
	class PanoseWidget;
}


class PanoseWidget : public QWidget {
	Q_OBJECT

	PanoseWidget(QWidget *parent = 0);
	static PanoseWidget * instance;
public:
	static PanoseWidget * getInstance();

	~PanoseWidget();

	void setFilter(const QMap<int, QList<int> >& filter);

protected:
	//    void changeEvent(QEvent *e);

private:
	Ui::PanoseWidget *m_ui;
	PanoseAttributeModel * attributeModel;
	PanoseValueModel * valueModel;

	int m_filterKey;
	QMap<int, QList<int> > m_filter;

private slots:
	void slotChangeAtrr(const QModelIndex& idx);
	void slotUpdateFilter(const QItemSelection & selected, const QItemSelection & deselected);

signals:
	void filterChanged(const QMap<int, QList<int> >&);

};

#endif // PANOSEWIDGET_H