/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef METAWIDGET_H
#define METAWIDGET_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "fmfontstrings.h"

class QStringListModel;
class QPushButton;
class QLineEdit;
class QComboBox;
class QHBoxLayout;

namespace Ui
{
class MetaWidget;
}

class MetaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetaWidget(QWidget *parent = nullptr);
    ~MetaWidget() override;

    QMap<int, QString> resultMap;

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::MetaWidget *const ui;

    static QStringListModel *mModel;
    static QStringList mList;
    QWidget *filterWidget = nullptr;
    QComboBox *filterCombo = nullptr;
    QLineEdit *filterLine = nullptr;
    QPushButton *filterButton = nullptr;
    QMap<QLineEdit *, FMFontDb::InfoItem> metFields;

Q_SIGNALS:
    void filterAdded();
    void Close();

private Q_SLOTS:
    void addFilter();
};

#endif // METAWIDGET_H
