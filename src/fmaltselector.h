/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMALTSELECTOR_H
#define FMALTSELECTOR_H

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QVariant>
#include <QWidget>

#include "fmaltcontext.h"

#include "ui_altselectorwidget.h"

class FMAltSelectorModel : public QAbstractItemModel
{
    Q_OBJECT

    class AltItem
    {
    public:
        enum Type {
            TEXT = 0,
            PARAGRAPH,
            WORD,
            CHUNK,
            GLYPH
        };

        AltItem(Type t, QVariant d)
            : T(t)
            , data(d)
        {
        }
        ~AltItem()
        {
            qDeleteAll(children);
        }
        const Type T;
        const QVariant data;
        AltItem *parent = nullptr;
        QList<int> alts;

        void addChild(AltItem *ai)
        {
            ai->parent = this;
            children.append(ai);
        }
        AltItem *child(int row)
        {
            return children.at(row);
        }
        int childCount()
        {
            return children.count();
        }
        int row()
        {
            if (parent)
                return parent->children.indexOf(const_cast<AltItem *>(this));
            return 0;
        }
        int columnCount()
        {
            if ((T == TEXT) || (T == GLYPH))
                return 2;
            return 1;
        }

    private:
        QList<AltItem *> children;
    };

    AltItem *rootItem = nullptr;

    // we need a delegate to display alt glyphs / selection widget
    class FMAltItemDelegate : public QAbstractItemDelegate
    {
        FMAltItemDelegate() = delete;
        const FMAltSelectorModel *pmodel = nullptr;

    public:
        explicit FMAltItemDelegate(FMAltSelectorModel *model);
        ~FMAltItemDelegate() override
        {
            delete pmodel;
        }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    };
    FMAltItemDelegate *altDelegate = nullptr;

public:
    FMAltSelectorModel();
    ~FMAltSelectorModel() override;

    void reModel(FMAltContext *ctx);

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    FMAltItemDelegate *AltDelegate()
    {
        return altDelegate;
    }
    friend class FMAltItemDelegate;
};

// the container of the view
class FMAltSelector : public QWidget, private Ui::AltSelectorWidget
{
    Q_OBJECT

    FMAltSelectorModel *m_model = nullptr;

public:
    explicit FMAltSelector(QWidget *parent);
    ~FMAltSelector() override = default;

public Q_SLOTS:
    void fillFromContext();
};

#endif // FMALTSELECTOR_H
