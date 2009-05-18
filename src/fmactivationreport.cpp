/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
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

#include "fmactivationreport.h"
#include <QDebug>
//#include <QTableWidget>
FMActivationReport::FMActivationReport(QWidget * parent, const QMap<QString,QString>& errorMap)
		:QDialog(parent)
{
//	QTableWidget errorTable;
	setupUi(this);
	errorTable->setSortingEnabled(false);
	int row(0);
	foreach(const QString& key, errorMap.keys())
	{
		qDebug()<<"EM"<<key<<errorMap[key];
		errorTable->insertRow (row);
		errorTable->setItem(row,0, new QTableWidgetItem(key));
		errorTable->setItem(row,1, new QTableWidgetItem(errorMap[key]));
		++row;
	}
	errorTable->sortByColumn(0,Qt::AscendingOrder);
	errorTable->setSortingEnabled(true);
	errorTable->resizeColumnsToContents();

}

