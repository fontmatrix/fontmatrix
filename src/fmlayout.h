/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMLAYOUT_H
#define FMLAYOUT_H

#include <QMap>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QThread>

#include "fmsharestruct.h"

class QGraphicsScene;
class FontItem;
class QGraphicsPixmapItem;
class QGraphicsPathItem;
class QGraphicsRectItem;
class QGraphicsProxyWidget;
class QProgressBar;
class QAction;
class QMenu;
class QMutex;
class FMLayOptWidget;
class QDialog;
class QGridLayout;

/**
 Finally, we want to layout text elsewhere than _in_ the font or _in_ the view.
 And it’s "peu ou prou" forced if we really want to experiment in Fontmatrix
 I hope it will not be too much more than just copy & paste from FontItem & MainViewWidget - pm
 It has been quite more! - pm
**/

#define INFINITE 99999999L

class FMLayout;

struct Node {
    struct ListItem {
        ListItem();
        ListItem(Node *N, double D);
        ~ListItem();
        Node *n = nullptr;
        double distance;

        // deletes n
        Q_DISABLE_COPY(ListItem)
    };

    Node(FMLayout *layoutEngine, int i);
    ~Node();

    QList<ListItem *> nodes;
    FMLayout *lyt = nullptr;
    int index = 0;

    // deletes the items of nodes
    Q_DISABLE_COPY(Node)

    bool hasNode(int idx);
    void nodes_clear();
    void nodes_insert(ListItem *v);

    void sPath(double dist, QList<int> curList, QList<int> &theList, double &theScore);
    int deepCount();

private:
    Node() = default;
};

class FMLayout : public QObject
{
    Q_OBJECT

public:
    explicit FMLayout(QGraphicsScene *scene, FontItem *font = nullptr, QRectF rect = QRectF());
    ~FMLayout() override;
    void doLayout(const QList<GlyphList> &spec, double fs, FontItem *font = nullptr);

private: // methods
    /// Build a graph on node
    virtual void doGraph();
    /// Build the good list of lines
    virtual void doLines();

    void clearCaches();

    void run();

public: // utils
    double distance(int start, int end, const GlyphList &gl, bool strip = false);
    int sepCount(int start, int end, const GlyphList &gl);
    void resetScene();

    bool isLayoutFinished()
    {
        return layoutIsFinished;
    }

    QList<int> breakList;
    QList<int> hyphenList;
    GlyphList theString;
    double lineWidth(int l);
    //		QMutex *layoutMutex;
    bool stopIt = false;
    int drawnLines = 0;

    void setContext(bool c);

public Q_SLOTS:
    void stopLayout();

private: // data
    // Argued
    //		static FMLayout *instance;
    bool contextIsMainThread;
    QGraphicsScene *theScene = nullptr;
    FontItem *theFont = nullptr;
    QList<GlyphList> paragraphs;
    QList<GlyphList> lines;
    /// how far from the left margin a line starts: the short last line of a right-to-left paragraph
    QMap<int, double> lineIndent;
    /// the direction of the paragraph being laid out, from its first strong character
    bool rightToLeftParagraph = false;
    QRectF theRect; // Not really argued now, will come soon
    QGraphicsRectItem *rules = nullptr;
    FMLayOptWidget *optionsWidget = nullptr;
    QGridLayout *optionLayout = nullptr;
    bool persistentScene;

    // built
    Node *node = nullptr;
    QList<int> indices;
    QList<QGraphicsPixmapItem *> pixList;
    QList<QGraphicsPathItem *> glyphList;
    QMap<int, QMap<int, double>> distCache;
    QMap<int, QMap<int, double>> stripCache;
    QMap<int, QMap<int, int>> sepCache;
    bool justRedraw = false;
    bool optionHasChanged;
    QPointF lastOrigine;

    // accessed
    bool processFeatures = false;
    QString script;
    bool processScript = false;
    double fontSize = 0.0;
    double adjustedSampleInter = 0.0;
    int textProgressionBlock = 0;
    int textProgressionLine = 0;
    bool deviceIndy = false;
    QPointF origine;

    bool layoutIsFinished;

public: // accessors
    [[nodiscard]] QRectF getRect() const
    {
        return theRect;
    }
    void setRect(const QRectF &r)
    {
        theRect = r;
    }
    void setProcessFeatures(bool theValue)
    {
        processFeatures = theValue;
    }
    void setScript(const QString &theValue)
    {
        script = theValue;
    }
    void setProcessScript(bool theValue)
    {
        processScript = theValue;
    }
    void setAdjustedSampleInter(double theValue);
    void setTextProgressionBlock(int theValue)
    {
        textProgressionBlock = theValue;
    }
    void setTextProgressionLine(int theValue)
    {
        textProgressionLine = theValue;
    }
    void setOrigine(const QPoint &theValue)
    {
        origine = theValue;
    }
    void setFontSize(bool theValue)
    {
        fontSize = theValue;
    }
    void setDeviceIndy(bool theValue)
    {
        deviceIndy = theValue;
    }
    //		void setTheScene ( QGraphicsScene* theValue , QRectF rect = QRectF());
    //		void setTheFont ( FontItem* theValue );
    void setPersistentScene(bool p)
    {
        persistentScene = p;
    }

private:
    /// Put lines on stage
    void doDraw();
    void endOfRun();
    void endOfParagraph();

    void slotOption(int v);

Q_SIGNALS:
    // needed if layout is executed outside the main (GUI) thread
    // receiver is expected to know with what font and on which scene;
    void drawPixmapForMe(int index, double fontsize, double x, double y);
    void drawBaselineForMe(double y);
    void clearScene();
    void objectWanted(QObject *);
    void updateLayout();
    void layoutFinished();
    void paragraphFinished();
    void paintFinished();

public:
    QWidget *optionDialog = nullptr;

    double FM_LAYOUT_NODE_SOON_F;
    double FM_LAYOUT_NODE_FIT_F;
    double FM_LAYOUT_NODE_LATE_F;
    double FM_LAYOUT_NODE_END_F;
    double FM_LAYOUT_HYPHEN_PENALTY;
    double FM_LAYOUT_MAX_COMPRESSION;
    QMenu *secretMenu = nullptr;
};

#endif
