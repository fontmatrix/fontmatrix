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

#include <QTreeWidgetItem>

PanoseWidget * PanoseWidget::instance = 0;

PanoseWidget::PanoseWidget(QWidget *parent) :
		QWidget(parent),
		m_ui(new Ui::PanoseWidget)
{
	m_ui->setupUi(this);
	attributeModel = new PanoseAttributeModel( this);
	valueModel = new PanoseValueModel( this);
	m_ui->attributeView->setModel(attributeModel);
	m_ui->valueView->setModel(valueModel);

	m_ui->pTree->hide();
	m_filter.clear();
	m_filterKey = 0;

	doConnect(true);
}

PanoseWidget::~PanoseWidget()
{
	delete m_ui;
}

PanoseWidget * PanoseWidget::getInstance()
{
	if(!instance)
	{
		instance = new PanoseWidget();
		Q_ASSERT(instance);
	}
	return instance;
}


void PanoseWidget::doConnect(const bool &c)
{
	if(c)
	{
		connect(m_ui->attributeView, SIGNAL(activated (const QModelIndex&)), this, SLOT(slotChangeAtrr(const QModelIndex&)));
		connect(m_ui->valueView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdateFilter(const QItemSelection & , const QItemSelection &)));
		connect(m_ui->pTree, SIGNAL(activated(QModelIndex)), this, SLOT(slotSelectAttr(QModelIndex)));
	}
	else
	{
		disconnect(m_ui->attributeView, SIGNAL(activated (const QModelIndex&)), this, SLOT(slotChangeAtrr(const QModelIndex&)));
		disconnect(m_ui->valueView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdateFilter(const QItemSelection & , const QItemSelection &)));
		disconnect(m_ui->pTree, SIGNAL(activated(QModelIndex)), this, SLOT(slotSelectAttr(QModelIndex)));
	}
}

void PanoseWidget::slotChangeAtrr(const QModelIndex& index)
{
	if(index.isValid())
	{
		m_filterKey = index.row();
		valueModel->setCat(m_filterKey);
		disconnect(m_ui->valueView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdateFilter(const QItemSelection & , const QItemSelection &)));
		m_ui->valueView->clearSelection();
		if(m_filter.contains(m_filterKey) && !m_filter[m_filterKey].isEmpty())
		{
			foreach(const int& r, m_filter[m_filterKey])
			{
				m_ui->valueView->selectionModel()->select(valueModel->index(r), QItemSelectionModel::Select);
			}
		}
		connect(m_ui->valueView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdateFilter(const QItemSelection & , const QItemSelection &)));
	}
}

void PanoseWidget::slotUpdateFilter(const QItemSelection & selected, const QItemSelection & deselected)
{
	m_filter.clear();
	QList<int> ns;
	foreach(const QModelIndex& i, m_ui->valueView->selectionModel()->selectedIndexes())
	{
		ns << i.row() + 2; // since "0" and "1" has been removed from the list
	}
	if(ns.count())
		m_filter[m_filterKey] = ns;
	else
		m_filter.remove(m_filterKey);

	m_ui->pTree->clear();
	foreach(const int& k, m_filter.keys())
	{
		QTreeWidgetItem *pItem(new QTreeWidgetItem(m_ui->pTree));
		pItem->setText(0, FontStrings::PanoseKeyName(FontStrings::PanoseKey(k)));
		foreach(const int& v, m_filter[k])
		{
			QTreeWidgetItem *item(new QTreeWidgetItem(pItem));
			item->setText(0, FontStrings::Panose().value( FontStrings::PanoseKey(k) )[v]);
		}
		pItem->setExpanded(true);
	}



	emit filterChanged();
	doConnect(false);
	m_ui->attributeView->clearSelection();
	m_ui->valueView->clearSelection();
	doConnect(true);
	hide();
}

void PanoseWidget::slotSelectAttr(const QModelIndex& idx)
{
	QModelIndex tmpIdx(idx);
	while(tmpIdx.parent().isValid())
		tmpIdx = tmpIdx.parent();

	const QString cs(tmpIdx.data(Qt::DisplayRole).toString());
	for(int i(0); i < m_ui->attributeView->model()->rowCount(); ++i)
	{
		const QModelIndex &cIdx(m_ui->attributeView->model()->index(i,0));
		if(cs == cIdx.data(Qt::DisplayRole).toString())
		{
			m_ui->attributeView->setCurrentIndex(cIdx);
			slotChangeAtrr(cIdx);
			return;
		}
	}

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
