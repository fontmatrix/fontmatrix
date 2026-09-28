/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FLOATINGWIDGET_H
#define FLOATINGWIDGET_H

#include <QPrintDialog>
#include <QPrinter>
#include <QString>
#include <QWidget>

class FloatingWidget : public QWidget
{
    Q_OBJECT

    explicit FloatingWidget(QWidget * = nullptr)
    {
    }

public:
    explicit FloatingWidget(const QString &f, const QString &typ, QWidget *parent = nullptr);
    ~FloatingWidget() override;

    [[nodiscard]] QString getActionName() const
    {
        return actionName;
    }

private:
    QString fName;
    QString fType;
    QString actionName;
    QString wTitle;

protected:
    QPrinter *printer = nullptr;
    QPrintDialog *printDialog = nullptr;
    bool event(QEvent *e) override;

Q_SIGNALS:
    void visibilityChange();
    void detached();

public Q_SLOTS:
    void activate(bool a);
    void ddetach(); // seems there's naming conflict with Qt4.7
};

#endif // FLOATINGWIDGET_H
