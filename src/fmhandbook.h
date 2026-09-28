/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMHANDBOOK_H
#define FMHANDBOOK_H

#include <QWidget>

class QAction;
class QTextBrowser;
class QUrl;

/**
 * The handbook in a window of the application, for the desktops where
 * KHelpCenter cannot show it: a Flatpak, which has none and whose files the
 * host cannot read, Windows, macOS, and a Linux where it is not installed.
 *
 * It reads the HTML the build generates from the same DocBook source
 * (FMPaths::HandbookFile()); QTextBrowser follows the links between the
 * chapters itself, and a link that leaves the handbook goes to the desktop.
 */
class FMHandbookWindow : public QWidget
{
    Q_OBJECT

public:
    /// shows the handbook, raising the window when it is already open
    static void showHandbook(QWidget *parent = nullptr);

private Q_SLOTS:
    void slotLinkClicked(const QUrl &url);
    void slotOpenInBrowser();

private:
    explicit FMHandbookWindow(const QString &file, QWidget *parent);

    QTextBrowser *m_browser = nullptr;
    QAction *m_back = nullptr;
    QAction *m_forward = nullptr;
    QString m_home;

    static FMHandbookWindow *m_window;
};

#endif
