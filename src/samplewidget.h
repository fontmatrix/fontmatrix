/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

#include "floatingwidget.h"
#include "fmotf.h"

#include <QByteArray>
#include <QFileInfo>
#include <QString>
#include <QThread>
#include <QTime>

class QGraphicsScene;
class FMLayout;
class QButtonGroup;
class QAbstractButton;
class QFileSystemWatcher;
class QTimer;
class SampleToolBar;
class FMVariationsWidget;
class QTreeWidgetItem;
class QStyledItemDelegate;
class FontItem;

namespace Ui
{
class SampleWidget;
}

constexpr int VIEW_PAGE_FREETYPE = 0;
constexpr int VIEW_PAGE_ABSOLUTE = 1;
constexpr int VIEW_PAGE_OPENTYPE = 3;
// #define VIEW_PAGE_SETTINGS 1
constexpr int VIEW_PAGE_SAMPLES = 4;
constexpr int VIEW_PAGE_VARIATIONS = 5;

class FMLayoutThread : public QThread
{
    FMLayout *pLayout = nullptr;
    QList<GlyphList> gl;
    double fontSize = 0.0;
    FontItem *font = nullptr;
    unsigned int fHinting = 0U;

public:
    void setLayout(FMLayout *l, const QList<GlyphList> &spec, double fs, FontItem *f, unsigned int hinting);
    void run() override;
};

class SampleWidget : public FloatingWidget
{
    Q_OBJECT

public:
    struct State {
        bool set;
        State()
            : set(false)
            , fontSize(0)
            , renderHinting(0)
        {
        }
        State(const QString &sn, double fs, unsigned int rh, const QString &sh, const QString &sc)
            : set(true)
            , sampleName(sn)
            , fontSize(fs)
            , renderHinting(rh)
            , shaper(sh)
            , script(sc)
        {
        }
        QString sampleName;
        double fontSize;
        unsigned int renderHinting; // 0 = No; 1 = Normal; 2 = Light
        QString shaper;
        QString script;
        [[nodiscard]] QByteArray toByteArray() const;
        State fromByteArray(QByteArray b);

        // private:
        // State operator= (const State&){}
    };

    static const QString Name;
    explicit SampleWidget(const QString &fid, QWidget *parent = nullptr);
    ~SampleWidget() override;

    [[nodiscard]] QGraphicsScene *textScene() const;
    [[nodiscard]] State state() const;
    void setState(const State &s);

protected:
    void changeEvent(QEvent *e) override;
    void refillSampleList();
    unsigned int hinting();

private:
    Ui::SampleWidget *const ui;
    SampleToolBar *sampleToolBar = nullptr;
    FMVariationsWidget *variationsWidget = nullptr;
    QTreeWidgetItem *uRoot = nullptr;
    QTreeWidgetItem *newSampleName = nullptr;
    QStyledItemDelegate *sampleNameEditor = nullptr;

    const QString fontIdentifier;
    QGraphicsScene *loremScene = nullptr;
    QGraphicsScene *ftScene = nullptr;
    FMLayout *textLayoutVect = nullptr;
    FMLayout *textLayoutFT = nullptr;
    //    QButtonGroup *radioRenderGroup;
    QButtonGroup *radioFTHintingGroup = nullptr;
    double sampleFontSize = 0.0;
    double sampleInterSize;
    double sampleRatio;
    int toolPanelWidth = 0;
    QFileSystemWatcher *sysWatcher = nullptr;
    QTimer *reloadTimer = nullptr;

    void createConnections();
    void removeConnections();

    void fillOTTree();
    OTFSet deFillOTTree();

    bool layoutForPrint;
    bool firstUpdateRequest = false;
    int firstUpdateRequestTimeStamp = 0;
    QTime layoutTime;
    QTimer *layoutTimer = nullptr;
    int layoutWait;
    FMLayoutThread *layoutThread = nullptr;
    bool layoutSwitch;
    int pixmapDrawn = 0;
#ifdef PLATFORM_APPLE
    QFileInfo fileInfo;
    qint64 fileLastModified = 0;
#endif
    void reSize(double fSize, double lSize)
    {
        sampleFontSize = fSize;
        sampleInterSize = lSize;
    }

private Q_SLOTS:
    void slotView();
    void doRender();
    void drawPixmap(int index, double fontsize, double x, double y);
    void clearFTScene();
    void slotUpdateSView();
    void slotZoom(int z);
    void slotUpdateRView();
    void slotSampleChanged();
    void slotLiveFontSize(double);
    void slotFeatureChanged();
    void slotDefaultOTF();
    void slotResetOTF();
    void slotFileChanged(const QString &);
    void slotReload();
    void slotScriptChange();

    void slotAddSample();
    void slotSampleNameEdited(QWidget *w);
    void slotRemoveSample();
    void slotEditSample();
    void slotUpdateSample();

    void slotShowSamples(bool);
    void slotShowOpenType(bool);
    void slotShowVariations(bool);

    void slotPrint();
    void slotDoPrinting();

    void saveState();

private:
    void drawBaseline(double y);
    void endLayout();
    //    void slotChangeViewPage(QAbstractButton* );
    //    void slotHintChanged(int);
    //    void slotChangeViewPageSetting(bool);
    void slotChangeScript();
    void slotProgressionChanged();
    void slotWantShape();

Q_SIGNALS:
    void stopLayout();
    void stateChanged();
};

#endif // SAMPLEWIDGET_H
