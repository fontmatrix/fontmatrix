/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIVATIONWIDGET_H
#define ACTIVATIONWIDGET_H

#include "floatingwidget.h"

namespace Ui
{
class ActivationWidget;
}

class ActivationWidgetItem;

class ActivationWidget : public FloatingWidget
{
    Q_OBJECT

public:
    static const QString Name;
    explicit ActivationWidget(const QString &familyName, QWidget *parent = nullptr);
    ~ActivationWidget() override;

protected:
    void changeEvent(QEvent *e) override;

private:
    const QString family;
    Ui::ActivationWidget *const ui;

    QList<ActivationWidgetItem *> items;
    void activateAll(bool c);

private Q_SLOTS:
    void slotActivate();
    void slotDeactivate();

Q_SIGNALS:
    void familyStateChanged();
};

#endif // ACTIVATIONWIDGET_H
