/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANOSEWIDGET_H
#define PANOSEWIDGET_H

#include <QItemSelection>
#include <QWidget>

class QTreeWidgetItem;

namespace Ui
{
class PanoseWidget;
}

class PanoseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PanoseWidget(QWidget *parent = nullptr);
    ~PanoseWidget() override;

    void setFilter(const QMap<int, QList<int>> &filter);
    [[nodiscard]] QMap<int, QList<int>> getFilter() const
    {
        return m_filter;
    }

protected:
    //    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *) override;

private:
    Ui::PanoseWidget *const m_ui;

    int m_filterKey;
    QMap<int, QList<int>> m_filter;

    void doConnect(const bool &c);

private Q_SLOTS:
    void slotSelect(QTreeWidgetItem *item, int column);

Q_SIGNALS:
    void filterChanged();
};

#endif // PANOSEWIDGET_H
