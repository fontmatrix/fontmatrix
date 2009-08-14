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

#ifndef PANOSEMODEL_H
#define PANOSEMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include <QList>
#include <QString>
#include <QMap>

class PanoseAttributeModel : public QAbstractListModel
{
public:
	PanoseAttributeModel(QObject * parent);

	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual int rowCount(const QModelIndex& parent) const;

private:
	QStringList m_names;
	QList<QIcon> m_icons;
};



class PanoseValueModel : public QAbstractListModel
{
public:
	PanoseValueModel(QObject * parent);

	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual int rowCount(const QModelIndex& parent) const;

	void setCat(const int& cat);

private:
	int m_cat;
	QMap<int, QList<QIcon> > m_icons;
	QMap<int, QStringList> m_names;
};

#endif // PANOSEMODEL_H
