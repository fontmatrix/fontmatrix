/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H

#include <QDialog>
#include <QList>
#include <QString>

namespace Ui
{
class FiltersDialog;
}

class FiltersDialogItem;
class FilterItem;
class FilterData;

class FiltersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FiltersDialog(const QList<FilterItem *> &currentFilter, QWidget *parent = nullptr);
    ~FiltersDialog() override;

private:
    Ui::FiltersDialog *const ui;

    void loadFilters();
    QString filterString(FilterData *d, bool first = false);
    QList<FiltersDialogItem *> items;

    // Lazy-initialised translated strings; static-init i18n() would run before
    // KLocalizedString::setApplicationDomain() and fall back to source text.
    static const QString &andOp();
    static const QString &notOp();
    static const QString &orOp();

Q_SIGNALS:
    void Filter(QString);
    void AddFilter(QString);
    void RemoveFilter(QString);

private Q_SLOTS:
    void slotAddFilter();
    void slotRemoveFilter(QString fs);
};

#endif // FILTERSDIALOG_H
