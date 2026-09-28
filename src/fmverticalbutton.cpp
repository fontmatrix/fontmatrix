/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmverticalbutton.h"

#include <QDebug>
#include <QEvent>
#include <QPainter>
// #include <QIcon>
// #include <QPaintEngine>

FMVerticalButton::FMVerticalButton(QWidget *parent)
    : QToolButton(parent)
{
    m_font.setBold(true);
}

FMVerticalButton::~FMVerticalButton() = default;

bool FMVerticalButton::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
        auto pe = reinterpret_cast<QPaintEvent *>(event);
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        if (m_text.isEmpty()) {
            m_text = text();
            setText(QString());
        }
        paintEvent(pe);
        QPainter p(this);
        m_font.setPixelSize(width() / 2);
        p.setFont(m_font);
        p.rotate(90);
        int vypos(qRound(static_cast<double>(width() - p.fontMetrics().ascent()) / 2.0));
        p.drawText(width() / 8, vypos * -1, m_text);
        return true;
    }
    return QToolButton::event(event);
}
