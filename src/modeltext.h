/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MODELTEXT_H
#define MODELTEXT_H

#include <QTextEdit>

class ModelText : public QTextEdit
{
    Q_OBJECT
public:
    explicit ModelText(QWidget *parent);
    ~ModelText() override = default;

protected:
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;

Q_SIGNALS:
    void insertContent();
};

#endif // MODELTEXT_H
