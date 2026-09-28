/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "activationwidgetitem.h"
#include "ui_activationwidgetitem.h"

#include "fmactivate.h"
#include "fmactivationreport.h"
#include "fmfontdb.h"
#include "fontitem.h"

ActivationWidgetItem::ActivationWidgetItem(const QString &fontID, QWidget *parent)
    : QWidget(parent)
    , fileName(fontID)
    , ui(new Ui::ActivationWidgetItem)
{
    ui->setupUi(this);
    FontItem *f(FMFontDb::DB()->Font(fileName));
    ui->styleName->setText(f->variant());
    ui->activatedStatus->setText(fileName);
    ui->activatedStatus->setChecked(f->isActivated());

    connect(ui->activatedStatus, &QCheckBox::toggled, this, &ActivationWidgetItem::activate);
}

ActivationWidgetItem::~ActivationWidgetItem()
{
    delete ui;
}

void ActivationWidgetItem::changeEvent(QEvent *e)
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

void ActivationWidgetItem::activate(bool a)
{
    FontItem *f(FMFontDb::DB()->Font(fileName));
    if (f == nullptr)
        return;
    if (a != f->isActivated()) {
        QList<FontItem *> fl;
        fl.clear();
        fl.append(f);
        FMActivate::getInstance()->errors();
        FMActivate::getInstance()->activate(fl, a);
        QMap<QString, QString> actErr(FMActivate::getInstance()->errors());
        if (actErr.count() > 0) {
            FMActivationReport ar(this, actErr);
            ar.exec();
        }
        Q_EMIT fontStateChanged();
    }
}

void ActivationWidgetItem::changeState(bool s)
{
    ui->activatedStatus->setChecked(s);
}

#include "moc_activationwidgetitem.cpp"
