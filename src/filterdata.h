/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERDATA_H
#define FILTERDATA_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>

class FilterItem;
class FontItem;

class FilterData : public QObject
{
    Q_OBJECT

public:
    FilterData();

    enum Index {
        Replace = 1,
        Or,
        And,
        Not,
        Text,
        UserIndex = 16
    };

    virtual void setData(int index, QVariant data, bool signalChange = false);
    [[nodiscard]] virtual QVariant data(int index) const;
    [[nodiscard]] virtual QString getText() const;
    [[nodiscard]] virtual QByteArray toByteArray() const;
    virtual void fromByteArray(const QByteArray &ba);
    virtual FilterItem *item();

    [[nodiscard]] virtual QString type() const = 0;
    virtual void operate() = 0;

protected:
    QMap<int, QVariant> vData;
    virtual void operateFilter(QList<FontItem *> fl);

private:
    QPointer<FilterItem> f;

Q_SIGNALS:
    void Operated();
    void Changed();
};

#endif // FILTERDATA_H
