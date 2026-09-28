/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmhandbook.h"
#include "fmpaths.h"
#include "fmportal.h"

#include <KLocalizedString>
#include <KSandbox>
#include <KStandardGuiItem>

#include <QAction>
#include <QDesktopServices>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QTextBrowser>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

FMHandbookWindow *FMHandbookWindow::m_window = nullptr;

void FMHandbookWindow::showHandbook(QWidget *parent)
{
    const QString file(FMPaths::HandbookFile());
    if (file.isEmpty())
        return;
    if (!m_window)
        m_window = new FMHandbookWindow(file, parent);
    m_window->show();
    m_window->raise();
    m_window->activateWindow();
}

FMHandbookWindow::FMHandbookWindow(const QString &file, QWidget *parent)
    : QWidget(parent, Qt::Window)
    , m_home(file)
{
    setWindowTitle(i18nc("@title:window", "Fontmatrix Handbook"));
    setAttribute(Qt::WA_DeleteOnClose, false);
    resize(900, 700);

    m_browser = new QTextBrowser(this);
    // the links between the chapters are ours to follow; what leaves the
    // handbook is the desktop's business
    m_browser->setOpenLinks(false);
    m_browser->setSearchPaths(QStringList(QFileInfo(file).absolutePath()));
    m_browser->setFrameShape(QFrame::NoFrame);

    QToolBar *bar = new QToolBar(this);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_back = bar->addAction(QIcon::fromTheme(QStringLiteral("go-previous")), i18nc("@action:intoolbar", "Back"));
    m_forward = bar->addAction(QIcon::fromTheme(QStringLiteral("go-next")), i18nc("@action:intoolbar", "Forward"));
    QAction *home = bar->addAction(QIcon::fromTheme(QStringLiteral("go-home")), i18nc("@action:intoolbar go to the table of contents", "Contents"));
    bar->addSeparator();
    QAction *browser = bar->addAction(QIcon::fromTheme(QStringLiteral("internet-web-browser")), i18nc("@action:intoolbar", "Open in Browser"));
    browser->setToolTip(i18nc("@info:tooltip", "Reads the same handbook in the web browser of the desktop."));
    m_back->setEnabled(false);
    m_forward->setEnabled(false);

    QPushButton *close = new QPushButton(this);
    KStandardGuiItem::assign(close, KStandardGuiItem::Close);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(close);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(bar);
    layout->addWidget(m_browser);
    layout->addLayout(buttons);
    buttons->setContentsMargins(6, 0, 6, 6);

    connect(m_browser, &QTextBrowser::anchorClicked, this, &FMHandbookWindow::slotLinkClicked);
    connect(m_browser, &QTextBrowser::backwardAvailable, m_back, &QAction::setEnabled);
    connect(m_browser, &QTextBrowser::forwardAvailable, m_forward, &QAction::setEnabled);
    connect(m_back, &QAction::triggered, m_browser, &QTextBrowser::backward);
    connect(m_forward, &QAction::triggered, m_browser, &QTextBrowser::forward);
    connect(home, &QAction::triggered, this, [this] {
        m_browser->setSource(QUrl::fromLocalFile(m_home));
    });
    connect(browser, &QAction::triggered, this, &FMHandbookWindow::slotOpenInBrowser);
    connect(close, &QPushButton::clicked, this, &QWidget::close);

    m_browser->setSource(QUrl::fromLocalFile(m_home));
}

void FMHandbookWindow::slotLinkClicked(const QUrl &url)
{
    // a chapter of the handbook is read here; a link that leaves it — the
    // licences at gnu.org, the project page — goes to the desktop, which in a
    // sandbox means the portal, and Qt asks it by itself
    if (url.isRelative() || url.isLocalFile())
        m_browser->setSource(url);
    else
        QDesktopServices::openUrl(url);
}

void FMHandbookWindow::slotOpenInBrowser()
{
    // in the sandbox the file itself goes to the desktop, as a browser of the
    // host cannot open a path inside the application
    if (KSandbox::isFlatpak() ? FMPortal::openRead(m_home, this) : QDesktopServices::openUrl(QUrl::fromLocalFile(m_home)))
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_home));
}

#include "moc_fmhandbook.cpp"
