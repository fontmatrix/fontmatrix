/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filteritem.h"
#include "filterdata.h"
#include "ui_filteritem.h"

FilterItem::FilterItem(FilterData *filter, QWidget *parent)
    : QWidget(parent)
    , d(filter)
    , ui(new Ui::FilterItem)
{
    ui->setupUi(this);

    QString ssheet;
    ssheet += QStringLiteral("QToolButton{border:none;}");
    ssheet += QStringLiteral("QToolButton:checked{border-bottom:2px solid black;}");
    ssheet += QStringLiteral("QToolButton:hover{background:white;}");
    //	ssheet += QString();
    //	ssheet += QString();
    //	ssheet += QString();
    //	ssheet += QString();
    this->setStyleSheet(ssheet);

    ui->filterLabel->setText(d->getText());
    ui->andButton->setChecked(d->data(FilterData::And).toBool());
    ui->noButton->setChecked(d->data(FilterData::Not).toBool());

    connect(ui->andButton, &QToolButton::clicked, this, &FilterItem::setAndMode);
    connect(ui->noButton, &QToolButton::clicked, this, &FilterItem::setNoMode);
    connect(ui->removeButton, &QToolButton::clicked, this, &FilterItem::remove);
}

void FilterItem::updateText()
{
    ui->filterLabel->setText(d->getText());
}

FilterItem::~FilterItem()
{
    delete ui;
    delete d;
}

void FilterItem::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FilterItem::setAndMode(bool c)
{
    d->setData(FilterData::Or, !c, true);
    d->setData(FilterData::And, c, true);
}

void FilterItem::setNoMode(bool c)
{
    d->setData(FilterData::Not, c, true);
}

void FilterItem::hideOperation(Operation op)
{
    if (op == AND) {
        if (d->data(FilterData::And).toBool()) {
            d->setData(FilterData::Or, true);
            d->setData(FilterData::And, false);
        }
        ui->andButton->hide();
    }
}

#include "moc_filteritem.cpp"
