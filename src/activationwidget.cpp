/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "activationwidget.h"
#include "ui_activationwidget.h"
#include <KLocalizedString>

#include "activationwidgetitem.h"
#include "fmactivate.h"
#include "fmactivationreport.h"
#include "fmfontdb.h"
#include "fmvariants.h"
#include "fontitem.h"

// Registry key used by FloatingWidgetsRegister; stable English identifier,
// never translated. User-facing labels come from the floating widget title.
const QString ActivationWidget::Name = QStringLiteral("Activation");

ActivationWidget::ActivationWidget(const QString &familyName, QWidget *parent)
    : FloatingWidget(familyName, Name, parent)
    , family(familyName)
    , ui(new Ui::ActivationWidget)
{
    ui->setupUi(this);
    QList<FontItem *> fl(FMVariants::Order(FMFontDb::DB()->FamilySet(family)));
    for (auto *f : std::as_const(fl)) {
        auto i(new ActivationWidgetItem(f->path(), this));
        ui->listLayout->addWidget(i);
        items.append(i);

        connect(i, &ActivationWidgetItem::fontStateChanged, this, &ActivationWidget::familyStateChanged);
    }

    connect(ui->activateAll, &QPushButton::clicked, this, &ActivationWidget::slotActivate);
    connect(ui->deactivateAll, &QPushButton::clicked, this, &ActivationWidget::slotDeactivate);
}

ActivationWidget::~ActivationWidget()
{
    delete ui;
}

void ActivationWidget::changeEvent(QEvent *e)
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

void ActivationWidget::slotActivate()
{
    activateAll(true);
}

void ActivationWidget::slotDeactivate()
{
    activateAll(false);
}

void ActivationWidget::activateAll(bool c)
{
    FMActivate::getInstance()->errors();
    FMActivate::getInstance()->activate(FMFontDb::DB()->FamilySet(family), c);
    QMap<QString, QString> actErr(FMActivate::getInstance()->errors());
    if (actErr.count() > 0) {
        FMActivationReport ar(this, actErr);
        ar.exec();
    }
    for (auto *i : std::as_const(items)) {
        i->changeState(c);
    }

    Q_EMIT familyStateChanged();
}

#include "moc_activationwidget.cpp"
