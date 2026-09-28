/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filtersdialogitem.h"
#include "ui_filtersdialogitem.h"

#include <KLocalizedString>
#include <KMessageBox>

FiltersDialogItem::FiltersDialogItem(const QString &name, const QString &f, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FiltersDialogItem)
    , filterName(name)
{
    ui->setupUi(this);
    QString ssheet;
    ssheet += QString("QToolButton{border:none;}");
    ssheet += QString("QToolButton:checked{border-bottom:2px solid black;}");
    ssheet += QString("QToolButton:hover{background:white;}");
    //	ssheet += QString();
    //	ssheet += QString();
    //	ssheet += QString();
    //	ssheet += QString();
    this->setStyleSheet(ssheet);
    //    setButtonsVisible(false);
    ui->filterName->setText(filterName);
    ui->filterName->setToolTip(f);
    //    ui->filters->setText(f);

    connect(ui->filterButton, &QToolButton::clicked, this, &FiltersDialogItem::slotFilter);
    connect(ui->removeButton, &QToolButton::clicked, this, &FiltersDialogItem::slotRemove);
}

FiltersDialogItem::~FiltersDialogItem()
{
    delete ui;
}

void FiltersDialogItem::slotFilter()
{
    Q_EMIT Filter(filterName);
}

void FiltersDialogItem::slotRemove()
{
    if (KMessageBox::warningContinueCancel(this,
                                           i18nc("@info", "Confirm deletion of filter:") + filterName,
                                           i18nc("@title:window", "Remove Filter"),
                                           KStandardGuiItem::remove())
        == KMessageBox::Continue)
        Q_EMIT Remove(filterName);
}

void FiltersDialogItem::setButtonsVisible(bool v)
{
    ui->filterButton->setVisible(v);
    ui->removeButton->setVisible(v);
}

void FiltersDialogItem::enterEvent(QEvent *)
{
    //	setButtonsVisible(true);
}

void FiltersDialogItem::leaveEvent(QEvent *)
{
    //	setButtonsVisible(false);
}

#include "moc_filtersdialogitem.cpp"
