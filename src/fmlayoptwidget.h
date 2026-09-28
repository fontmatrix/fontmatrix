/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMLAYOPTWIDGET_H
#define FMLAYOPTWIDGET_H

#include <ui_layoutoptions.h>

class FMLayOptWidget : public QWidget, private Ui::LayoutOptionWidget
{
    Q_OBJECT
public:
    enum V {
        BEFORE = 1,
        EXACT,
        AFTER,
        END,
        HYPHEN,
        SPACE,
        MAX
    };

    explicit FMLayOptWidget(QWidget *parent = nullptr);

    int vToInt(V v)
    {
        return v;
    }
    void setRange(V v, int min, int max);
    void setValue(V v, int value);
    void setValue(V v, double value);
    double getValue(V v);

private Q_SLOTS:
    void bChanged(int cv);
    void exChanged(int cv);
    void aChanged(int cv);
    void enChanged(int cv);
    void hChanged(int cv);
    void sChanged(int cv);

    void bEdited();
    void exEdited();
    void aEdited();
    void enEdited();
    void hEdited();
    void sEdited();

Q_SIGNALS:
    /// Indicates which slider has been changed
    /// It’s up to the receiver to ask the new value;
    void valueChanged(int);
};

#endif
