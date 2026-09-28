/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAINVIEWWIDGET_H
#define MAINVIEWWIDGET_H

#include <ui_mainview.h>

#include <QElapsedTimer>
#include <QIcon>
#include <QStringList>

class QGraphicsScene;
class typotek;
class FontItem;
class QTextEdit;
class QGridLayout;
class QTreeWidgetItem;
class QGraphicsRectItem;
class QButtonGroup;
struct OTFSet;
class FMLayout;
class FMPreviewModel;
class QTimer;

/**
MainViewWidget inherits from an ui designed.

    @author Pierre Marchand <pierre@oep-h.com>
*/
class MainViewWidget : public QWidget, private Ui::MainView
{
    Q_OBJECT

public:
    explicit MainViewWidget(QWidget *parent);

    ~MainViewWidget() override;

private:
    QStringList ord;
    QStringList fields;
    typotek *typo = nullptr;
    QString faceIndex;
    QString lastIndex;
    //		QList<FontItem*> currentFonts; *moved to FMFontDB*
    QList<FontItem *> orderedCurrentFonts;
    //		QString sampleText;
    QGridLayout *tagLayout = nullptr;
    QString currentOrdering;
    FontItem *theVeryFont = nullptr;
    bool fontsetHasChanged;
    bool activateByFamilyOnly;
    bool m_forceReloadSelection;
    QString quickSearchString;
    QElapsedTimer quickSearchTime;
    QTimer *quickSearchTimer = nullptr;
    int quickSearchWait;

    void doConnect();
    void disConnect();
    void allActivation(bool act);
    void activation(QList<FontItem *> fit, bool act);

    QString curItemName;

    bool renderingLock = false;

    QIcon iconPS1;
    QIcon iconTTF;
    QIcon iconOTF;

    FMPreviewModel *previewModel = nullptr;

public Q_SLOTS:
    void slotFontDbChanged();
    bool slotFontSelectedByName(const QString &fname);
    void slotPreviewUpdateSize(int w);
    void slotShowFamily(const QModelIndex &familyIdx);
    void slotQuitFamily();
    void slotActivateAll();

public:
    void slotOrderingChanged(QString s);
    void slotDesactivateAll();
    void slotRemoveCurrentItem();

private Q_SLOTS:
    void slotQuickSearch(const QString &text);
    void slotEndQuickSearch();
    void slotSelectFromQuickSearch();

Q_SIGNALS:
    void faceChanged();
    void listChanged();

public:
    QString defaultOrd()
    {
        return ord.at(0);
    }
    QList<FontItem *> curFonts();
    void setCurFonts(QList<FontItem *> flist);
    FontItem *selectedFont()
    {
        return theVeryFont;
    }
    // The selected font or, in the list, the one of the highlighted tile
    FontItem *selectedOrCurrentFont();

    QString sampleName();
    //		void displayWelcomeMessage();

    void addFilterToCrumb(QString filter);
    void setCrumb(QString text = QString());

    void saveSplitterState();
    void restoreSplitterState();

    void forceReloadSelection();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif
