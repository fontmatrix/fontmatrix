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

#include "panosemodel.h"
#include "parallelcoor.h"
#include "fmpaths.h"
#include "fmfontstrings.h"

#include <QSettings>
#include <QDir>
#include <QFile>

/// ATTRIBUTE MODEL

PanoseAttributeModel::PanoseAttributeModel(QObject * parent)
		:QAbstractListModel(parent)
{
	const QMap< FontStrings::PanoseKey, QMap<int, QString> >& p(FontStrings::Panose());
	QSettings settings;
	QString defaultDir(FMPaths::ResourcesDir() + "Panose/Icons");
	QString pDir(settings.value("Panose/IconDir", defaultDir).toString() + QDir::separator());
	foreach(const FontStrings::PanoseKey& k, p.keys())
	{
		QString fn(pDir + QString::number(k) + QDir::separator() + "attribute.png");
		if(QFile::exists(fn))
			m_icons << QIcon(fn);
		else
			m_icons << QIcon();
		m_names << FontStrings::PanoseKeyName(k);
	}
}



QVariant PanoseAttributeModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();
	if(Qt::DisplayRole == role)
	{
		return m_names.at(index.row());
	}
	else if(Qt::DecorationRole == role)
	{
		return m_icons.at(index.row());
	}

	return QVariant();

}

int PanoseAttributeModel::rowCount(const QModelIndex& parent) const
{
	return m_icons.count();
}

/// END OF ATTRIBUTE MODEL


/// VALUE MODEL

PanoseValueModel::PanoseValueModel( QObject * parent)
		:QAbstractListModel(parent)
{
	const QMap< FontStrings::PanoseKey, QMap<int, QString> >& p(FontStrings::Panose());
	QSettings settings;
	QString defaultDir(FMPaths::ResourcesDir() + "Panose/Icons");
	QString pDir(settings.value("Panose/IconDir", defaultDir).toString() + QDir::separator());

	foreach(const FontStrings::PanoseKey& k, p.keys())
	{
		foreach(const int& v, p[k].keys())
		{
			if(v > 1) // We do not want "Any" and "No Fit"
			{
				QString fn(pDir + QString::number(k) + QDir::separator() + QString::number(v) +".png");
				if(QFile::exists(fn))
					m_icons[k] << QIcon(fn);
				else
					m_icons[k] << QIcon();
				m_names[k] << p[k][v];
			}
		}
	}
	m_cat = FontStrings::firstPanoseKey();
}

void PanoseValueModel::setCat(const int& cat)
{
	if(cat != m_cat)
	{
		m_cat = cat;
		emit layoutChanged();
	}
}


int PanoseValueModel::rowCount(const QModelIndex& parent) const
{
	return m_names[m_cat].count();
}

QVariant PanoseValueModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();

	if(Qt::DisplayRole == role)
	{
		if(m_icons[m_cat].at(index.row()).isNull())
			return m_names[m_cat].at(index.row());
		return QString();
	}
	else if(Qt::DecorationRole == role)
	{
		return m_icons[m_cat].at(index.row());
	}
	else if(Qt::ToolTipRole == role)
	{
		return m_names[m_cat].at(index.row());
	}
	return QVariant();
}


/// END OF VALUE MODEL



