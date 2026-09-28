/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "modeltext.h"

#include <QDebug>
#include <QMimeData>

ModelText::ModelText(QWidget *parent)
    : QTextEdit(parent)
{
}

bool ModelText::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->hasFormat(QStringLiteral("application/x-qabstractitemmodeldatalist"))) {
        return true;
    } else
        return QTextEdit::canInsertFromMimeData(source);
}

void ModelText::insertFromMimeData(const QMimeData *source)
{
    if (source->hasFormat(QStringLiteral("application/x-qabstractitemmodeldatalist"))) {
        Q_EMIT insertContent();
    } else
        QTextEdit::insertFromMimeData(source);
}

#include "moc_modeltext.cpp"
