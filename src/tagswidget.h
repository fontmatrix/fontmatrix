/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include "ui_tagswidget.h"

class FontItem;
class TagsWidget_ListModel;

class TagsWidget : public QWidget, private Ui::tagsWidget
{
    Q_OBJECT

    TagsWidget_ListModel *model = nullptr;

public:
    explicit TagsWidget(QWidget *parent);
    ~TagsWidget() override;

    void prepare(QList<FontItem *> fonts);

private Q_SLOTS:
    void slotNewTag();
    void slotActRemovetag();
};

#endif // TAGSWIDGET_H
