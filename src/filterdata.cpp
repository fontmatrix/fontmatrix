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

#include "filterdata.h"
#include "fmfontdb.h"
#include "filteritem.h"

#include <QDataStream>

#include <QBitArray>
#include <QBitmap>
#include <QByteArray>
#include <QChar>
#include <QIcon>
#include <QImage>
#include <QLocale>
#include <QMatrix>
#include <QTransform>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QRegExp>
#include <QSizeF>
#include <QString>
#include <QTime>
#include <QUrl>


FilterData::FilterData()
{
	vData.clear();
	// default operation
	vData.insert(Replace, false);
	vData.insert(Or, true);
	vData.insert(And, false);
	vData.insert(Not, false);
}


void FilterData::setData(int index, QVariant data)
{
	vData.insert(index, data);
}

QVariant FilterData::data(int index) const
{
	return vData.value(index);
}


QString FilterData::getText() const
{
	return vData.value(Text, QString("*")).toString();
}

FilterItem* FilterData::item()
{
	if(f.isNull())
		f = new FilterItem(this);
	return f.data();
}

void FilterData::operateFilter(QList<FontItem *>fl)
{
	QList<FontItem*> tmpList = fl;
	QList<FontItem*> negList;
	QList<FontItem*> queList;

	bool negate(vData[Not].toBool());
	bool queue(vData[And].toBool());
	bool append(vData[Or].toBool());

	FMFontDb* fmdb(FMFontDb::DB());

	if(queue)
	{
		queList = fmdb->getFilteredFonts();
	}
	if(negate)
		negList = fmdb->AllFonts();

	if(!append)
		fmdb->clearFilteredFonts();

	if(negate)
	{
		if(queue)
		{
			foreach(FontItem* f, negList)
			{
				if(!tmpList.contains(f) && queList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else if(append)
		{
			foreach(FontItem* f, tmpList)
			{
				if(!fmdb->isFiltered(f) && !tmpList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, negList)
			{
				if(!tmpList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
	}
	else // not negate
	{
		if(queue)
		{
			foreach(FontItem* f, tmpList)
			{
				if(queList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else if(append)
		{
			foreach(FontItem* f, tmpList)
			{
				if(!fmdb->isFiltered(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, tmpList)
			{
				fmdb->insertFilteredFont(f);
			}
		}
	}

	emit Operated();
}


QByteArray FilterData::toByteArray() const
{
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	foreach(int idx, vData.keys())
	{
		QVariant::Type t(vData.value(idx).type());
		QVariant v(vData[idx]);
		ds << idx << t;
		switch(t)
		{
			// We keep a large subset of supported types, but well, it's rather optimistic.
		case QVariant::BitArray : { ds <<  v.value<QBitArray>(); break; }
		case QVariant::Bool : { ds <<  v.value<bool>(); break; }
		case QVariant::ByteArray : { ds <<  v.value<QByteArray>(); break; }
		case QVariant::Char : { ds <<  v.value<QChar>(); break; }
		case QVariant::Double : { ds <<  v.value<double>(); break; }
		case QVariant::Icon : { ds <<  v.value<QIcon>(); break; }
		case QVariant::Image : { ds <<  v.value<QImage>(); break; }
		case QVariant::Int : { ds <<  v.value<int>(); break; }
		case QVariant::Locale : { ds <<  v.value<QLocale>(); break; }
		case QVariant::LongLong : { ds <<  v.value<qlonglong>(); break; }
		case QVariant::Matrix : { ds <<  v.value<QMatrix>(); break; }
		case QVariant::Transform : { ds <<  v.value<QTransform>(); break; }
		case QVariant::Pixmap : { ds <<  v.value<QPixmap>(); break; }
		case QVariant::PointF : { ds <<  v.value<QPointF>(); break; }
		case QVariant::RectF : { ds <<  v.value<QRectF>(); break; }
		case QVariant::RegExp : { ds <<  v.value<QRegExp>(); break; }
		case QVariant::SizeF : { ds <<  v.value<QSizeF>(); break; }
		case QVariant::String : { ds <<  v.value<QString>(); break; }
		case QVariant::Time : { ds <<  v.value<QTime>(); break; }
		case QVariant::UInt : { ds <<  v.value<uint>(); break; }
		case QVariant::ULongLong : { ds <<  v.value<qulonglong>(); break; }
		case QVariant::Url : { ds <<  v.value<QUrl>(); break; }
		}

	}
	return ba;
}

void FilterData::fromByteArray(const QByteArray &ba)
{
	QDataStream ds(ba);
	int idx(0);
	ds >> idx;
	while(idx != 0)
	{
		QVariant::Type t;
		ds >> t;
		QVariant v(t);
		switch(t)
		{
		case QVariant::BitArray : { QBitArray data; ds >> data ; v.setValue(data); break; }
		case QVariant::Bitmap : { QBitmap data; ds >> data ; v.setValue(data); break; }
		case QVariant::Bool : { bool data; ds >> data ; v.setValue(data); break; }
		case QVariant::ByteArray : { QByteArray data; ds >> data ; v.setValue(data); break; }
		case QVariant::Char : { QChar data; ds >> data ; v.setValue(data); break; }
		case QVariant::Double : { double data; ds >> data ; v.setValue(data); break; }
		case QVariant::Icon : { QIcon data; ds >> data ; v.setValue(data); break; }
		case QVariant::Image : { QImage data; ds >> data ; v.setValue(data); break; }
		case QVariant::Int : { int data; ds >> data ; v.setValue(data); break; }
		case QVariant::Locale : { QLocale data; ds >> data ; v.setValue(data); break; }
		case QVariant::LongLong : { qlonglong data; ds >> data ; v.setValue(data); break; }
		case QVariant::Matrix : { QMatrix data; ds >> data ; v.setValue(data); break; }
		case QVariant::Transform : { QTransform data; ds >> data ; v.setValue(data); break; }
		case QVariant::Pixmap : { QPixmap data; ds >> data ; v.setValue(data); break; }
		case QVariant::PointF : { QPointF data; ds >> data ; v.setValue(data); break; }
		case QVariant::RectF : { QRectF data; ds >> data ; v.setValue(data); break; }
		case QVariant::RegExp : { QRegExp data; ds >> data ; v.setValue(data); break; }
		case QVariant::SizeF : { QSizeF data; ds >> data ; v.setValue(data); break; }
		case QVariant::String : { QString data; ds >> data ; v.setValue(data); break; }
		case QVariant::Time : { QTime data; ds >> data ; v.setValue(data); break; }
		case QVariant::UInt : { uint data; ds >> data ; v.setValue(data); break; }
		case QVariant::ULongLong : { qulonglong data; ds >> data ; v.setValue(data); break; }
		case QVariant::Url : { QUrl data; ds >> data ; v.setValue(data); break; }

		}
		vData.insert(idx, v);
	}

}
