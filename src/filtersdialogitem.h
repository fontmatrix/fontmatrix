/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERSDIALOGITEM_H
#define FILTERSDIALOGITEM_H

#include <QString>
#include <QWidget>

namespace Ui
{
class FiltersDialogItem;
}

class FiltersDialogItem : public QWidget
{
    Q_OBJECT

public:
    explicit FiltersDialogItem(const QString &name, const QString &f, QWidget *parent = nullptr);
    ~FiltersDialogItem() override;

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *) override;

private:
    Ui::FiltersDialogItem *const ui;

    void setButtonsVisible(bool v);
    QString filterName;

private Q_SLOTS:
    void slotFilter();
    void slotRemove();

Q_SIGNALS:
    void Filter(QString);
    void Remove(QString);
};

#endif // FILTERSDIALOGITEM_H
