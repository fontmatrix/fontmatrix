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

#include "panosewidget.h"
#include "ui_panosewidget.h"
#include "panosemodel.h"
#include "fmfontstrings.h"
#include "fmpaths.h"

#include <QTreeWidgetItem>
#include <QDir>
#include <QSettings>
#include <QIcon>
#include <QColor>


PanoseWidget::PanoseWidget(QWidget *parent) :
		QWidget(parent),
		m_ui(new Ui::PanoseWidget)
{
	m_ui->setupUi(this);

	m_filter.clear();
	m_filterKey = 0;


	const QMap< FontStrings::PanoseKey, QMap<int, QString> >& p(FontStrings::Panose());
	QSettings settings;
	QString defaultDir(FMPaths::ResourcesDir() + "Panose/Icons");
	QString pDir(settings.value("Panose/IconDir", defaultDir).toString() + QDir::separator());

	foreach(const FontStrings::PanoseKey& k, p.keys())
	{
		QString fn(pDir + QString::number(k) + QDir::separator() + "attribute.png");
		QTreeWidgetItem  * pItem(new QTreeWidgetItem(m_ui->pTree));

		pItem->setText(0, FontStrings::PanoseKeyName(k));
		pItem->setData(0,Qt::UserRole,k);
		if(QFile::exists(fn))
			pItem->setIcon(0, QIcon(fn));

		foreach(const int& v, p[k].keys())
		{
			if(v > 1) // We do not want "Any" and "No Fit"
			{
				QString fn2(pDir + QString::number(k) + QDir::separator() + QString::number(v) +".png");

				QTreeWidgetItem * item(new QTreeWidgetItem(pItem));
				item->setText(0, p[k][v]);
				item->setData(0,Qt::UserRole,v);
				item->setForeground(0, QColor(qrand() % 255, qrand() % 255, qrand() % 255));
				if(QFile::exists(fn2))
					item->setIcon(0, QIcon(fn2));
			}
		}
	}

	connect(m_ui->pTree, SIGNAL(itemClicked(QTreeWidgetItem * , int )), this, SLOT(slotSelect(QTreeWidgetItem * , int )));
}

PanoseWidget::~PanoseWidget()
{
	delete m_ui;
}


void PanoseWidget::doConnect(const bool &c)
{
	if(c)
	{
		connect(m_ui->pTree, SIGNAL(clicked(QTreeWidgetItem * , int )), this, SLOT(slotSelect(QTreeWidgetItem * , int )));
	}
	else
	{
		disconnect(m_ui->pTree, SIGNAL(clicked(QTreeWidgetItem * , int )), this, SLOT(slotSelect(QTreeWidgetItem * , int )));
	}
}

void PanoseWidget::slotSelect(QTreeWidgetItem *item, int column)
{
	if(column > 0 )
		return;
	if(item->childCount() > 0)
		return;
	int pValue(item->parent()->data(0,Qt::UserRole).toInt());
	int cValue(item->data(0,Qt::UserRole).toInt());

	m_filter.clear();
	m_filter.insert(pValue,QList<int>() << cValue);

	emit filterChanged();
}

void PanoseWidget::setFilter(const QMap<int, QList<int> >& filter)
{
	m_filter = filter;
}

//void PanoseWidget::changeEvent(QEvent *e)
//{
//    QWidget::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        m_ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}

void PanoseWidget::closeEvent(QCloseEvent *)
{
	hide();
}
