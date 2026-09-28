/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMGLYPHSVIEW_H
#define FMGLYPHSVIEW_H

#include <QGraphicsView>
#include <QRectF>

/**
    @author Pierre Marchand <pierre@oep-h.com>
*/
class FMGlyphsView : public QGraphicsView
{
    Q_OBJECT
public:
    enum ViewState {
        AllView,
        SingleView
    };

    explicit FMGlyphsView(QWidget *parent);
    ~FMGlyphsView() override;

    void setState(ViewState s);
    ViewState state()
    {
        return m_state;
    }

    QRectF visibleSceneRect();
    bool lock();
    void unlock();

private:
    ViewState m_state;
    bool m_lock;
    bool m_oper;

private Q_SLOTS:
    void slotViewMoved(int);

Q_SIGNALS:
    /**
        forward new width, allowing FontItem::renderAll() to adjust the number of columns
    */
    void refit(int);
    void pleaseShowSelected();
    void pleaseShowAll();
    void pleaseUpdateMe();
    void pleaseUpdateSingle();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    FMGlyphsView(const FMGlyphsView &) = delete;
    FMGlyphsView &operator=(const FMGlyphsView &) = delete;
};

#endif
