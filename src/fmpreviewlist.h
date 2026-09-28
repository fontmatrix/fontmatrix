/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMPREVIEWLIST_H
#define FMPREVIEWLIST_H

#include <QAbstractListModel>
#include <QIconEngine>
#include <QListView>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPoint>

class FontItem;
class MainViewWidget;
class QListView;

// Rather than fighting against Qt to not resize our icons, draw them ourselves.
class FMPreviewIconEngine : public QIconEngine
{
public:
    enum Activation {
        NotActivated,
        Activated,
        PartlyActivated
    };
    FMPreviewIconEngine();
    ~FMPreviewIconEngine() override;
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override;
    void setActivation(Activation a)
    {
        activatedFont = a;
    }
    [[nodiscard]] QIconEngine *clone() const override;

private:
    // only clone() copies an engine
    FMPreviewIconEngine(const FMPreviewIconEngine &) = default;
    FMPreviewIconEngine &operator=(const FMPreviewIconEngine &) = delete;

    QPixmap m_p;
    Activation activatedFont;
    static bool initState;
    static QPen pen;
    static QVector<QRgb> m_selPalette;
    static QRgb activatedColor;
    static QRgb deactivatedColor;
    static QRgb partlyActivatedColor;

    QVector<QRgb> actualSelPalette(const QVector<QRgb> &orig);
};

class FMPreviewView : public QListView
{
    Q_OBJECT
public:
    explicit FMPreviewView(QWidget *parent = nullptr);
    ~FMPreviewView() override = default;
    [[nodiscard]] int getUsedWidth() const
    {
        return usedWidth;
    }
    // n = 1 for a single column layout (list); n = 2 for multi columns layout
    void setNumCol(int n)
    {
        columns = n;
    }
    bool moveTo(const QString &fname); // fname can be the begining of a fancy name

protected:
    void resizeEvent(QResizeEvent *event) override;
    QPoint startDragPoint;
    bool dragFlag;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    int usedWidth = 0;
    int columns;

public Q_SLOTS:
    void updateLayout();

public:
    void setCurrentFont(const QString &name);

Q_SIGNALS:
    void widthChanged(int);
    void keyPressed(QString);
};

class FMPreviewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum PreviewItemRole {
        PathRole = Qt::UserRole + 1,
        InstanceRole ///< the named instance of a variable font the row shows, -1 for the font itself
    };

    FMPreviewModel(QObject *pa, FMPreviewView *wPa, QList<FontItem *> db = QList<FontItem *>());
    // returns a preview
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    // returns flags for items
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    // returns the number of items
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    void resetBase(QList<FontItem *> db);
    QList<FontItem *> getBase();

    void setFamilyMode(bool f)
    {
        familyMode = f;
    }
    void setSpecString(const QString &s)
    {
        specString = s;
    }

    [[nodiscard]] bool getFamilyMode() const
    {
        return familyMode;
    }

    /// a variable font is shown as its named instances, one row each (the family view)
    void setInstanceMode(bool i)
    {
        instanceMode = i;
    }
    /// the row of the font, at the named instance when it has rows for them
    [[nodiscard]] QModelIndex indexOf(FontItem *font, int instance) const;

private:
    FMPreviewView *m_view = nullptr;
    QList<FontItem *> base;
    QString specString;
    QString styleTooltipName;
    QString styleTooltipTags;
    QString styleTooltipPath;
    bool familyMode;
    bool instanceMode = false;
    struct Row {
        FontItem *font = nullptr;
        int instance = -1;
    };
    QList<Row> rows; ///< in instance mode, what each row shows

public Q_SLOTS:
    void dataChanged();
};

#endif
