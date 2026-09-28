/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "floatingwidgettoolbar.h"
#include "ui_floatingwidgettoolbar.h"

#include "floatingwidget.h"

#include <QAction>
#include <QMenu>

FloatingWidgetToolBar::FloatingWidgetToolBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FloatingWidgetToolBar)
    , noClose(false)
    , isDetached(false)
{
    ui->setupUi(this);

    connect(ui->closeButton, &QToolButton::clicked, this, &FloatingWidgetToolBar::Close);
    connect(ui->hideButton, &QToolButton::clicked, this, &FloatingWidgetToolBar::Hide);
    connect(ui->printButton, &QToolButton::clicked, this, &FloatingWidgetToolBar::Print);
    connect(ui->detachButton, &QToolButton::clicked, this, &FloatingWidgetToolBar::setDetached);
    setupMenu();
}

FloatingWidgetToolBar::~FloatingWidgetToolBar()
{
    delete ui;
}

void FloatingWidgetToolBar::changeEvent(QEvent *e)
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

void FloatingWidgetToolBar::setupMenu()
{
    if (isDetached) {
        if (!noClose)
            ui->closeButton->show();
        else
            ui->closeButton->hide();
        ui->hideButton->show();
        ui->detachButton->hide();
    } else {
        ui->closeButton->hide();
        ui->hideButton->hide();
        ui->detachButton->show();
    }
    ui->printButton->show();
}

void FloatingWidgetToolBar::setDetached()
{
    isDetached = true;
    setupMenu();
    Q_EMIT Detach();
}

void FloatingWidgetToolBar::setNoClose(bool c)
{
    noClose = c;
    setupMenu();
}

#include "moc_floatingwidgettoolbar.cpp"
