/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Holds ParallelCoorView & ParallelCoorDataSet

#ifndef PARALLELCOOR_H
#define PARALLELCOOR_H

#include <iterator>

#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

/**
    TODO describe _precisely_ how datas are stored
*/
using ParallelCoorDataType = QList<QList<int>>;
class ParallelCoorDataSet : public QMap<QString, QList<QString>>
{
public:
    static const QString FieldSep;

    ParallelCoorDataSet();
    ~ParallelCoorDataSet();

    // get Field of a complete "path"
    QString getField(const QString &p)
    {
        return p.mid(0, p.indexOf(FieldSep) - 1);
    }
    // get Place of a complete "path"
    QString getPlace(const QString &p)
    {
        return p.right(p.indexOf(FieldSep));
    }

private:
    // descriptions of "dimensions"
    QMap<QString, QString> m_categoryDescriptions;

    // descriptions of "places". keys are dimension + FieldSep + place
    QMap<QString, QString> m_valueDescriptions;

    ParallelCoorDataType m_data;

public:
    // put here set/get methods
    void setCategoryDescriptions(const QMap<QString, QString> &theValue);
    [[nodiscard]] QMap<QString, QString> getCategoryDescriptions() const;
    void setValueDescriptions(const QMap<QString, QString> &theValue);
    [[nodiscard]] QMap<QString, QString> getValueDescriptions() const;
    void setData(const ParallelCoorDataType &theValue);
    [[nodiscard]] ParallelCoorDataType getData() const;

    /// compatibility layer :)
    QPair<QString, QList<QString>> at(int idx)
    {
        if (idx < count()) {
            auto it(constBegin());
            std::advance(it, idx);
            return QPair<QString, QList<QString>>(it.key(), it.value());
        }
        return QPair<QString, QList<QString>>();
    }
};

class ParallelCoorFieldItem : public QGraphicsSimpleTextItem
{
public:
    ParallelCoorFieldItem(QString text, QGraphicsView *pcv, QGraphicsItem *parent = nullptr);
    ~ParallelCoorFieldItem() override = default;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsView *pview = nullptr;
};

class ParallelCoorValueItem : public QGraphicsSimpleTextItem
{
public:
    ParallelCoorValueItem(QString text, QGraphicsView *pcv, QGraphicsItem *parent = nullptr);
    ~ParallelCoorValueItem() override = default;

    void hoverEnter();
    void hoverLeave();
    void click(int mod = 0);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsView *pview = nullptr;
};

class ParallelCoorBarItem : public QGraphicsLineItem
{
public:
    ParallelCoorBarItem(const QString &field, QGraphicsView *pcv, QGraphicsItem *parent = nullptr);
    ~ParallelCoorBarItem() override = default;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsView *pview = nullptr;
    QString attachedField;
};

class ParallelCoorMarkItem : public QGraphicsPathItem
{
public:
    ParallelCoorMarkItem(ParallelCoorValueItem *relative, QGraphicsView *pcv, QGraphicsItem *parent = nullptr);
    ~ParallelCoorMarkItem() override = default;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsView *pview = nullptr;
    ParallelCoorValueItem *value = nullptr;
};

class ParallelCoorView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ParallelCoorView(QWidget *parent = nullptr);
    explicit ParallelCoorView(ParallelCoorDataSet *dataset, QWidget *parent = nullptr);
    ~ParallelCoorView() override;

    void selectField(const QString &field);
    [[nodiscard]] bool matchFilter(QList<int> list) const;

    // put here set/get methods
    void setDataSet(ParallelCoorDataSet *theValue);
    [[nodiscard]] ParallelCoorDataSet *getDataSet() const;
    void setFilter(const QMap<QString, QStringList> &theValue);
    [[nodiscard]] QMap<QString, QStringList> getFilter() const;
    QString filterAsString();
    void setCurrentField(const QString &theValue);
    [[nodiscard]] QString getCurrentField() const;

public:
    void updateGraphic();

protected:
    // main method that redraws all
    virtual void redraw();

    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
Q_SIGNALS:
    void selectedField(const QString &);
    void filterChanged();

private:
    void doConnect();
    // At some point the dataset will send signals
    // when data is updated. Thus, beeing a descendant of
    // QObject, it will "inherit" the Q_DISABLE_COPY macro.
    ParallelCoorDataSet *m_dataSet = nullptr;

    // if empty, all items of the dataset are shown.
    // else, only datas that match the filter
    QMap<QString, QStringList> m_filter;
    QMap<int, QList<int>> cfilter; // something like a compiled version of the filter

    // hm, not very meaningful but it can be useful.
    QString m_currentField;

    // the very privates with no access methods.
    QList<ParallelCoorValueItem *> valueLabels;
    QList<ParallelCoorFieldItem *> fieldLabels;
    QList<QGraphicsLineItem *> vertices;
    QList<ParallelCoorBarItem *> bars;
    QList<ParallelCoorMarkItem *> marks;

    struct Units {
        double hunit = 0.0;
        double wunit = 0.0;
        double XOffset = 0.0;
        double YOffset = 0.0;
        double H = 0.0;
        double W = 0.0;
        int C = 0;
        double step = 0.0;
        Units() = default;
        Units(int width, int height, int count);
    };

    Units units;
    enum ItemList {
        AllList,
        BarList,
        VerticeList,
        FieldList,
        ValueList
    };

    void initPensAndBrushes();

    void cleanLists(ItemList il = AllList);
    void drawBars();
    void drawVertices();
    void drawFields();
    void drawValues();

    QSize controlSize;

private:
    void slotSaveColors();

public:
    static QMap<QString, QPen> pens;
    static QMap<QString, QBrush> brushes;
    static QPainterPath markPath;
};

#endif
