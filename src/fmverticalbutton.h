/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMVERTICALBUTTON_H
#define FMVERTICALBUTTON_H

#include <QFont>
#include <QToolButton>

class FMVerticalButton : public QToolButton
{
public:
    explicit FMVerticalButton(QWidget *parent);
    ~FMVerticalButton() override;

protected:
    bool event(QEvent *event) override;

private:
    QString m_text;
    QFont m_font;
};

#endif
