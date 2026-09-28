/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmactivationreport.h"
#include "fontmatrix_debug.h"
#include <QDebug>
// #include <QTableWidget>
FMActivationReport::FMActivationReport(QWidget *parent, const QMap<QString, QString> &errorMap)
    : QDialog(parent)
{
    //	QTableWidget errorTable;
    setupUi(this);
    errorTable->setSortingEnabled(false);
    int row(0);
    for (const auto errorMapKeys = errorMap.keys(); const auto &key : errorMapKeys) {
        qCDebug(FONTMATRIX_LOG) << "EM" << key << errorMap[key];
        errorTable->insertRow(row);
        errorTable->setItem(row, 0, new QTableWidgetItem(key));
        errorTable->setItem(row, 1, new QTableWidgetItem(errorMap[key]));
        ++row;
    }
    errorTable->sortByColumn(0, Qt::AscendingOrder);
    errorTable->setSortingEnabled(true);
    errorTable->resizeColumnsToContents();
}
