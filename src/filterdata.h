/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
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

#ifndef FILTERDATA_H
#define FILTERDATA_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QPointer>
#include <QByteArray>

class FilterItem;
class FontItem;

class FilterData : public QObject
{
	Q_OBJECT

public:
	FilterData();

	enum Index{
		Replace = 1,
		Or,
		And,
		Not,
		Text,
		UserIndex = 16
	};

	virtual void setData(int index, QVariant data, bool signalChange = false);
	virtual QVariant data(int index) const;
	virtual QString getText() const;
	virtual QByteArray toByteArray() const;
	virtual void fromByteArray(const QByteArray& ba);
	virtual FilterItem* item();

	virtual QString type() const = 0;
	virtual void operate() = 0;

protected:
	QMap<int, QVariant> vData;
	virtual void operateFilter(QList<FontItem*> fl);

private:
	QPointer<FilterItem> f;

signals:
	void Operated();
	void Changed();
};

#endif // FILTERDATA_H
