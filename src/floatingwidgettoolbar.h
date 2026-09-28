/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FLOATINGWIDGETTOOLBAR_H
#define FLOATINGWIDGETTOOLBAR_H

#include <QWidget>

class QMenu;
class QAction;

namespace Ui
{
class FloatingWidgetToolBar;
}

class FloatingWidgetToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingWidgetToolBar(QWidget *parent = nullptr);
    ~FloatingWidgetToolBar() override;

    void setNoClose(bool c);

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::FloatingWidgetToolBar *const ui;

    bool noClose;
    bool isDetached;

    void setupMenu();

public Q_SLOTS:
    void setDetached();

Q_SIGNALS:
    void Close();
    void Hide();
    void Print();
    void Detach();
};

#endif // FLOATINGWIDGETTOOLBAR_H
