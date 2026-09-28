/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FONTCOMPAREWIDGET_H
#define FONTCOMPAREWIDGET_H

#include "ui_comparewidget.h"

class FontCompareWidget : public QWidget, private Ui::CompareWidget
{
    Q_OBJECT

    static FontCompareWidget *instance;
    explicit FontCompareWidget(QWidget *parent);

public:
    static FontCompareWidget *getInstance();
    ~FontCompareWidget() override;

private:
    QString curFont;
    uint curcode = 0U;
    bool neverUsed;

    void doconnect();
    void dodisconnect();
    void resetElements();

    void initColors();

Q_SIGNALS:
    // Emitted whenever this window is shown or hidden, including when the
    // user closes it from its own title bar. typotek's checkable Compare
    // action is toggled on by the action itself, so without this it stays
    // checked after the window goes away.
    void visibilityChanged();

protected:
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

private Q_SLOTS:
    void addFont();
    void removeFont();
    void showChange();
    void fillChange(int newIdx);
    void pointsChange();
    void controlsChange();
    void metricsChange();
    void offsetChange(int o);
    void characterChange(int v);
    void characterBoxChange(int i);
    void fontChange(QListWidgetItem *witem, QListWidgetItem *olditem = nullptr);
    void syncChange(bool checked);
};

#endif
