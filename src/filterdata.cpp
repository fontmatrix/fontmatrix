/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterdata.h"
#include "filteritem.h"
#include "fmfontdb.h"

#include <QDataStream>
#include <QIODevice>

#include <QBitArray>
#include <QBitmap>
#include <QByteArray>
#include <QChar>
#include <QIcon>
#include <QImage>
#include <QLocale>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QTime>
#include <QTransform>
#include <QUrl>

FilterData::FilterData()
{
    vData.clear();
    // default operation
    vData.insert(Replace, false);
    vData.insert(Or, true);
    vData.insert(And, false);
    vData.insert(Not, false);
}

void FilterData::setData(int index, QVariant data, bool signalChange)
{
    vData.insert(index, data);
    if (signalChange)
        Q_EMIT Changed();
}

QVariant FilterData::data(int index) const
{
    return vData.value(index);
}

QString FilterData::getText() const
{
    return vData.value(Text, QStringLiteral("*")).toString();
}

FilterItem *FilterData::item()
{
    if (f.isNull())
        f = new FilterItem(this);
    return f.data();
}

void FilterData::operateFilter(QList<FontItem *> fl)
{
    QList<FontItem *> sourceList = fl;
    QList<FontItem *> notList;
    QList<FontItem *> andList;

    bool notOp(vData.value(Not).toBool());
    bool andOp(vData.value(And).toBool());
    bool orOp(vData.value(Or).toBool());

    FMFontDb *fmdb(FMFontDb::DB());

    if (andOp) {
        andList = fmdb->getFilteredFonts();
    }
    if (notOp)
        notList = fmdb->AllFonts();

    if (!orOp)
        fmdb->clearFilteredFonts();

    if (notOp) {
        if (andOp) {
            for (auto *f : std::as_const(notList)) {
                if (!sourceList.contains(f) && andList.contains(f))
                    fmdb->insertFilteredFont(f);
            }
        } else {
            for (auto *f : std::as_const(notList)) {
                if (!sourceList.contains(f))
                    fmdb->insertFilteredFont(f);
            }
        }
    } else // not notOp
    {
        if (andOp) {
            for (auto *f : sourceList) {
                if (andList.contains(f))
                    fmdb->insertFilteredFont(f);
            }
        } else {
            for (auto *f : sourceList) {
                fmdb->insertFilteredFont(f);
            }
        }
    }

    Q_EMIT Operated();
}

QByteArray FilterData::toByteArray() const
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (const auto vDataKeys = vData.keys(); int idx : vDataKeys) {
        int t = vData.value(idx).typeId();
        QVariant v(vData[idx]);
        ds << idx << t;
        switch (t) {
            // We keep a large subset of supported types, but well, it's rather optimistic.
        case QMetaType::QBitArray: {
            ds << v.value<QBitArray>();
            break;
        }
        case QMetaType::Bool: {
            ds << v.value<bool>();
            break;
        }
        case QMetaType::QByteArray: {
            ds << v.value<QByteArray>();
            break;
        }
        case QMetaType::QChar: {
            ds << v.value<QChar>();
            break;
        }
        case QMetaType::Double: {
            ds << v.value<double>();
            break;
        }
        case QMetaType::QIcon: {
            ds << v.value<QIcon>();
            break;
        }
        case QMetaType::QImage: {
            ds << v.value<QImage>();
            break;
        }
        case QMetaType::Int: {
            ds << v.value<int>();
            break;
        }
        case QMetaType::QLocale: {
            ds << v.value<QLocale>();
            break;
        }
        case QMetaType::LongLong: {
            ds << v.value<qlonglong>();
            break;
        }
        case QMetaType::QTransform: {
            ds << v.value<QTransform>();
            break;
        }
        case QMetaType::QPixmap: {
            ds << v.value<QPixmap>();
            break;
        }
        case QMetaType::QPointF: {
            ds << v.value<QPointF>();
            break;
        }
        case QMetaType::QRectF: {
            ds << v.value<QRectF>();
            break;
        }
        case QMetaType::QSizeF: {
            ds << v.value<QSizeF>();
            break;
        }
        case QMetaType::QString: {
            ds << v.value<QString>();
            break;
        }
        case QMetaType::QTime: {
            ds << v.value<QTime>();
            break;
        }
        case QMetaType::UInt: {
            ds << v.value<uint>();
            break;
        }
        case QMetaType::ULongLong: {
            ds << v.value<qulonglong>();
            break;
        }
        case QMetaType::QUrl: {
            ds << v.value<QUrl>();
            break;
        }
        }
    }
    return ba;
}

void FilterData::fromByteArray(const QByteArray &ba)
{
    QDataStream ds(ba);
    int idx(0);
    ds >> idx;
    while (idx != 0) {
        int t;
        ds >> t;
        QVariant v;
        switch (t) {
        case QMetaType::QBitArray: {
            QBitArray data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QBitmap: {
            QBitmap data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::Bool: {
            bool data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QByteArray: {
            QByteArray data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QChar: {
            QChar data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::Double: {
            double data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QIcon: {
            QIcon data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QImage: {
            QImage data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::Int: {
            int data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QLocale: {
            QLocale data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::LongLong: {
            qlonglong data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QTransform: {
            QTransform data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QPixmap: {
            QPixmap data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QPointF: {
            QPointF data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QRectF: {
            QRectF data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QSizeF: {
            QSizeF data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QString: {
            QString data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QTime: {
            QTime data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::UInt: {
            uint data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::ULongLong: {
            qulonglong data;
            ds >> data;
            v.setValue(data);
            break;
        }
        case QMetaType::QUrl: {
            QUrl data;
            ds >> data;
            v.setValue(data);
            break;
        }
        }
        vData.insert(idx, v);

        ds >> idx;
    }
}

#include "moc_filterdata.cpp"
