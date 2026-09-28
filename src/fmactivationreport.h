/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMACTIVATIONREPORT_H
#define FMACTIVATIONREPORT_H

#include "ui_activationreportdialog.h"
#include <QMap>

class FMActivationReport : public QDialog, private Ui::ActivationReportDialog
{
public:
    FMActivationReport(QWidget *parent, const QMap<QString, QString> &errorMap);
};

#endif // FMACTIVATIONREPORT_H
