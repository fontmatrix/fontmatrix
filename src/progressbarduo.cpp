/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "progressbarduo.h"
#include "ui_progressbarduo.h"

ProgressBarDuo::ProgressBarDuo(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProgressBarDuo)
{
    ui->setupUi(this);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ProgressBarDuo::Canceled);
}

ProgressBarDuo::~ProgressBarDuo()
{
    delete ui;
}

void ProgressBarDuo::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ProgressBarDuo::setLabel(const QString &s, int n)
{
    QLabel *l = nullptr;

    switch (n) {
    case 0:
        l = ui->Label0;
        break;
    case 1:
        l = ui->Label1;
        break;
    default:
        break;
    }
    if (l)
        l->setText(s);
}

void ProgressBarDuo::setValue(int value, int n)
{
    QProgressBar *p = nullptr;
    switch (n) {
    case 0:
        p = ui->Bar0;
        break;
    case 1:
        p = ui->Bar1;
        break;
    default:
        break;
    }
    if (p)
        p->setValue(value);
}

void ProgressBarDuo::setMax(int max, int n)
{
    QProgressBar *p = nullptr;
    switch (n) {
    case 0:
        p = ui->Bar0;
        break;
    case 1:
        p = ui->Bar1;
        break;
    default:
        break;
    }
    if (p)
        p->setMaximum(max);
}

#include "moc_progressbarduo.cpp"
