/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QRectF>
#include <QWidget>

class QGraphicsScene;

namespace Ui
{
class PlayWidget;
}

class PlayWidget : public QWidget
{
    Q_OBJECT

    static PlayWidget *instance;
    explicit PlayWidget();

public:
    ~PlayWidget() override;
    static PlayWidget *getInstance();
    double playFontSize();
    QRectF getMaxRect();
    void clearSelection();
    QGraphicsScene *getPlayScene()
    {
        return playScene;
    }

Q_SIGNALS:
    // Emitted whenever this window is shown or hidden, including when the
    // user closes it from its own title bar. typotek's checkable Playground
    // action is toggled on by the action itself, so without this it stays
    // checked after the window goes away.
    void visibilityChanged();

protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *) override;
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

private:
    Ui::PlayWidget *const ui;
    QGraphicsScene *playScene = nullptr;

private Q_SLOTS:
    void slotZoom(int z);
    void print();
};

#endif // PLAYWIDGET_H
