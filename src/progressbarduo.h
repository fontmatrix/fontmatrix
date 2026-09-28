/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROGRESSBARDUO_H
#define PROGRESSBARDUO_H

#include <QDialog>

namespace Ui
{
class ProgressBarDuo;
}

class ProgressBarDuo : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressBarDuo(QWidget *parent = nullptr);
    ~ProgressBarDuo() override;

    void setLabel(const QString &s, int n);
    void setValue(int value, int n);
    void setMax(int max, int n);

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::ProgressBarDuo *const ui;

Q_SIGNALS:
    void Canceled();
};

#endif // PROGRESSBARDUO_H
