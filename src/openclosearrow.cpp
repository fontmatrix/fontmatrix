/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "openclosearrow.h"

OpenCloseArrow::OpenCloseArrow(QWidget *parent, bool open)
    : QLabel(parent)
    , openingState(open)
{
    //	setOpText();
}

OpenCloseArrow::~OpenCloseArrow() = default;

void OpenCloseArrow::mouseReleaseEvent(QMouseEvent *)
{
    openingState = !openingState;
    setOpText();
    Q_EMIT openChanged(openingState);
}

void OpenCloseArrow::changeOpen(bool t)
{
    openingState = t;
    setOpText();
    Q_EMIT openChanged(openingState);
}

void OpenCloseArrow::setOpText()
{
    if (baseText.isEmpty())
        baseText = text();
    QFont f(font());
    if (openingState) {
        f.setBold(true);
        setFont(f);
        setText(QStringLiteral("%1").arg(baseText));
    } else {
        f.setBold(false);
        setFont(f);
        setText(QStringLiteral("%1").arg(baseText));
    }
}

#include "moc_openclosearrow.cpp"
