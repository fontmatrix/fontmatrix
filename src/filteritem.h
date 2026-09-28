/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERITEM_H
#define FILTERITEM_H

#include <QWidget>

class FilterData;

namespace Ui
{
class FilterItem;
}

class FilterItem : public QWidget
{
    Q_OBJECT

    FilterData *d = nullptr;
    QString text;

public:
    enum Operation {
        AND,
        OR,
        NOT
    };

    explicit FilterItem(FilterData *filter, QWidget *parent = nullptr);
    ~FilterItem() override;

    FilterData *filter()
    {
        return d;
    }
    void hideOperation(Operation op);
    // to call after the text of the filter has changed
    void updateText();

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::FilterItem *const ui;

Q_SIGNALS:
    void remove();

private Q_SLOTS:
    void setAndMode(bool c);
    void setNoMode(bool c);
};

#endif // FILTERITEM_H
