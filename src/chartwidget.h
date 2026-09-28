/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include "floatingwidget.h"

class QGraphicsScene;
class FontItem;
class QGraphicsRectItem;

namespace Ui
{
class ChartWidget;
}

class ChartWidget : public FloatingWidget
{
    Q_OBJECT

public:
    static const QString Name;
    explicit ChartWidget(const QString &fid, const QString &block, QWidget *parent = nullptr);
    ~ChartWidget() override;

    QString currentBlock();
    void selectBlock(const QString &uname);

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::ChartWidget *const ui;
    const QString fontIdentifier;

    QGraphicsScene *abcScene = nullptr;
    int fancyGlyphInUse;
    int fancyGlyphData = 0;
    QString unMapGlyphName;
    QString allMappedGlyphName;
    bool uRangeIsNotEmpty;
    QGraphicsRectItem *curGlyph = nullptr;

    void createConnections();
    void removeConnections();
    void fillUniPlanesCombo(FontItem *item);

private Q_SLOTS:
    void slotShowOneGlyph();
    void slotShowAllGlyph();
    void slotAdjustGlyphView(int width);
    void slotUpdateGView();
    void slotUpdateGViewSingle();
    void slotPlaneSelected(int);
    void slotShowULine(bool);
    void slotSearchCharName();

    void slotPrint();
    void slotDoPrinting();
};

#endif // CHARTWIDGET_H
