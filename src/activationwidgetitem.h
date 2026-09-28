/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIVATIONWIDGETITEM_H
#define ACTIVATIONWIDGETITEM_H

#include <QWidget>

namespace Ui
{
class ActivationWidgetItem;
}

class ActivationWidgetItem : public QWidget
{
    Q_OBJECT

public:
    explicit ActivationWidgetItem(const QString &fontID, QWidget *parent = nullptr);
    ~ActivationWidgetItem() override;

    void changeState(bool s);

protected:
    void changeEvent(QEvent *e) override;

private:
    const QString fileName;
    Ui::ActivationWidgetItem *const ui;

private Q_SLOTS:
    void activate(bool a);

Q_SIGNALS:
    void fontStateChanged();
};

#endif // ACTIVATIONWIDGETITEM_H
