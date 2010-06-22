/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>

class QGraphicsScene;
class FontItem;
class QGraphicsRectItem;

namespace Ui {
    class ChartWidget;
}

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(const QString& fid, QWidget *parent = 0);
    ~ChartWidget();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ChartWidget *ui;

    QGraphicsScene *abcScene;
    FontItem *theVeryFont;
    int fancyGlyphInUse;
    int fancyGlyphData;
    QString unMapGlyphName;
    QString allMappedGlyphName;
    bool uRangeIsNotEmpty;
    QGraphicsRectItem *curGlyph;


    void fillUniPlanesCombo(FontItem* item);

private slots:
    void slotShowOneGlyph();
    void slotShowAllGlyph();
    void slotAdjustGlyphView(int width);
    void slotUpdateGView();
    void slotUpdateGViewSingle();
    void slotPlaneSelected(int);
    void slotShowULine(bool);
    void slotSearchCharName();

};

#endif // CHARTWIDGET_H
