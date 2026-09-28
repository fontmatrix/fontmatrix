/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TTTABLEVIEW_H
#define TTTABLEVIEW_H

#include "ui_tttablewidget.h"

class FontItem;
class QTreeWidgetItem;

class TTTableView : public QWidget, private Ui::TTTableWidget
{
    Q_OBJECT
    enum Tfield_p {
        NAME = 0,
        DESCRIPTION = 1,
        SIZE = 2
    };

    FontItem *m_font = nullptr;
    QList<QTreeWidgetItem *> twiList;
    QVector<uint8_t> m_data;

public:
    explicit TTTableView(FontItem *font, QWidget *parent = nullptr);
    ~TTTableView() override;

private:
    QByteArray curTable;

private Q_SLOTS:
    void updateHexView();
    void exportHex();
};

#endif
