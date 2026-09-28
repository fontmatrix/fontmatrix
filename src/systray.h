/*
    SPDX-FileCopyrightText: 2007 Riku Leino <riku@scribus.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QMap>
#include <QObject>

class QAction;
class QMenu;
class KStatusNotifierItem;
class typotek;

class Systray : public QObject
{
    Q_OBJECT

public:
    Systray();
    ~Systray() override;

    bool isVisible();
    bool hasActivateAll();
    bool allConfirmation();
    bool tagsConfirmation();

public Q_SLOTS:
    void hide();
    void updateTagMenu(const QStringList &nameOfFontWhichCausedThisUpdate);

public:
    void show();
    void slotSetVisible(bool isVisible);
    void slotSetActivateAll(bool isVisible);
    void newTag(QString);
    void deleteTag(const QString &name);
    void requireAllConfirmation(bool doRequire);
    void requireTagsConfirmation(bool doRequire);

private Q_SLOTS:
    void slotActivateAll();
    void slotDeactivateAll();
    void slotMinimize();
    void slotRestore();
    void slotTagMenuClicked(QAction *action);
    void slotQuit();
    void slotPrepareMenu();

private:
    bool confirmAll = false;
    bool confirmTags = false;

    void createActions();
    void createTrayIcon();
    void createTagMenu();

    QAction *activateAllAction = nullptr;
    QAction *deactivateAllAction = nullptr;
    QAction *minimizeAction = nullptr;
    QAction *restoreAction = nullptr;
    QAction *quitAction = nullptr;

    KStatusNotifierItem *trayIcon = nullptr;
    QMenu *trayIconMenu = nullptr;
    QMenu *tagMenu = nullptr;
    // 	QMenu           *tagSetMenu;

    QMap<QString, QAction *> tagActions;

    static typotek *ttek;

    bool showAllConfirmation;
    bool showTagsConfirmation;

    Systray(const Systray &) = delete;
    Systray &operator=(const Systray &) = delete;

    // 	protected:
    // 		bool eventFilter ( QObject * watched, QEvent * event ) ;
};

#endif
