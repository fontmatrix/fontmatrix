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

#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

#include "floatingwidget.h"
#include <QString>
#include "fmotf.h"

class QGraphicsScene;
class FMLayout;
class QButtonGroup;
class QAbstractButton;
class QFileSystemWatcher;
class QTimer;

namespace Ui {
    class SampleWidget;
}

#define VIEW_PAGE_FREETYPE 0
#define VIEW_PAGE_ABSOLUTE 1
#define VIEW_PAGE_OPENTYPE 0
#define VIEW_PAGE_SETTINGS 1
#define VIEW_PAGE_SAMPLES  2

class SampleWidget : public FloatingWidget
{
    Q_OBJECT

public:
    explicit SampleWidget(const QString& fid, FloatingWidget *parent = 0);
    ~SampleWidget();

    QGraphicsScene* textScene()const;

protected:
    void changeEvent(QEvent *e);

    void refillSampleList();
    unsigned int hinting();

private:
    Ui::SampleWidget *ui;

    const QString fontIdentifier;
    QGraphicsScene *loremScene;
    QGraphicsScene *ftScene;
    FMLayout *textLayoutVect;
    FMLayout *textLayoutFT;
    QButtonGroup *radioRenderGroup;
    QButtonGroup *radioFTHintingGroup;
    double sampleFontSize;
    double sampleInterSize;
    double sampleRatio;
    int toolPanelWidth;
    QFileSystemWatcher *sysWatcher;
    QTimer *reloadTimer;

    void fillOTTree();
    OTFSet deFillOTTree();

    bool layoutForPrint;

    void reSize(double fSize, double lSize){sampleFontSize = fSize; sampleInterSize = lSize;}

private slots:
    void slotView(bool needDeRendering = false);
    void slotChangeViewPage(QAbstractButton* );
    void slotHintChanged(int);
    void slotChangeViewPageSetting(bool);
    void slotUpdateSView();
    void slotZoom(int z);
    void slotUpdateRView();
    void slotSampleChanged();
    void slotEditSampleText();
    void slotLiveFontSize();
    void slotFeatureChanged();
    void slotDefaultOTF();
    void slotResetOTF();
    void slotChangeScript();
    void slotProgressionChanged();
    void slotWantShape();
    void slotFileChanged(const QString&);
    void slotReload();

    void slotPrint();


signals:
    void stopLayout();



};

#endif // SAMPLEWIDGET_H
