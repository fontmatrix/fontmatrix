/*
    SPDX-FileCopyrightText: 2007 Riku Leino <riku@scribus.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "systray.h"
#include "fmconfig.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "mainviewwidget.h"
#include "typotek.h"
#include <KLocalizedString>
#include <KStatusNotifierItem>
#include <QDebug>
#include <QMenu>

typotek *Systray::ttek = nullptr;

Systray::Systray()
{
    createActions();
    createTrayIcon();
    createTagMenu();

    connect(trayIconMenu, &QMenu::aboutToShow, this, &Systray::slotPrepareMenu);

    showAllConfirmation = FMConfig::value(QStringLiteral("Systray/AllConfirmation"), true).toBool();
    showTagsConfirmation = FMConfig::value(QStringLiteral("Systray/TagsConfirmation"), false).toBool();

    slotSetActivateAll(FMConfig::value(QStringLiteral("Systray/ActivateAllVisible"), false).toBool());

    const QIcon trayThemed = QIcon::fromTheme(QStringLiteral("fontmatrix-tray"), QIcon(QStringLiteral(":/fontmatrix_systray_icon.png")));
    trayIcon->setIconByPixmap(trayThemed);
    trayIcon->setToolTipIconByPixmap(trayThemed);
    trayIcon->setToolTipTitle(QStringLiteral("Fontmatrix"));

    if (FMConfig::value(QStringLiteral("Systray/Visible"), false).toBool())
        trayIcon->setStatus(KStatusNotifierItem::Active);
    else
        trayIcon->setStatus(KStatusNotifierItem::Passive);
}

Systray::~Systray() = default;

void Systray::slotSetVisible(bool isVisible)
{
    trayIcon->setStatus(isVisible ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);
    FMConfig::setValue(QStringLiteral("Systray/Visible"), isVisible);
}

void Systray::slotSetActivateAll(bool isVisible)
{
    activateAllAction->setVisible(isVisible);
    deactivateAllAction->setVisible(isVisible);
    FMConfig::setValue(QStringLiteral("Systray/ActivateAllVisible"), isVisible);
}

void Systray::show()
{
    trayIcon->setStatus(KStatusNotifierItem::Active);
}

void Systray::hide()
{
    trayIcon->setStatus(KStatusNotifierItem::Passive);
}

void Systray::slotMinimize()
{
    ttek->showMinimized();
}

void Systray::slotRestore()
{
    ttek->showNormal();
}

void Systray::slotActivateAll()
{
    //	ttek->theMainView->slotViewAll();
    //	if (showAllConfirmation) {
    //		bool wasVisible = ttek->isVisible();
    //		if (!wasVisible)
    //			ttek->show();
    //		ttek->slotActivateCurrents();
    //		if (!wasVisible)
    //			ttek->hide();
    //	} else
    //		ttek->theMainView->slotActivateAll();

    //	disconnect(tagMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotTagMenuClicked(QAction*)));
    //	QList<QAction*> tags = tagActions.values();
    //	for (auto* a : tags) {
    //		a->setChecked(true);
    //	}
    //	connect(tagMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotTagMenuClicked(QAction*)));
}

void Systray::slotDeactivateAll()
{
    //	ttek->theMainView->slotViewAll();
    //	if (showAllConfirmation) {
    //		bool wasVisible = ttek->isVisible();
    //		if (!wasVisible)
    //			ttek->show();
    //		ttek->slotDeactivateCurrents();
    //		if (!wasVisible)
    //			ttek->hide();
    //	} else
    //		ttek->theMainView->slotDesactivateAll();
    //	disconnect(tagMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotTagMenuClicked(QAction*)));
    //	QList<QAction*> tags = tagActions.values();
    //	for (auto* a : tags) {
    //		a->setChecked(false);
    //	}
    //	connect(tagMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotTagMenuClicked(QAction*)));
}

void Systray::slotTagMenuClicked(QAction *)
{
    // Deactivate the feature atm
    //	action->setIcon(QIcon ());
    //	QString name = action->text();
    //	if (name.isEmpty())
    //		return;

    //	if (!action->isChecked()) { // deactivate based on the tag name
    //		ttek->theMainView->slotFilterTag(name);
    //		if (showTagsConfirmation) {
    //			bool wasVisible = ttek->isVisible();
    //			if (!wasVisible)
    //				ttek->show();
    //			ttek->slotDeactivateCurrents();
    //			if (!wasVisible)
    //				ttek->hide();
    //		} else
    //			ttek->theMainView->slotDesactivateAll();
    //	} else { // activate based on the tag name
    //		ttek->theMainView->slotFilterTag(name);
    //		if (showTagsConfirmation) {
    //			bool wasVisible = ttek->isVisible();
    //			if (!wasVisible)
    //				ttek->show();
    //			ttek->slotActivateCurrents();
    //			if (!wasVisible)
    //				ttek->hide();
    //		} else
    //			ttek->theMainView->slotActivateAll();
    //	}
}

void Systray::slotQuit()
{
    // Route through typotek::slotQuit() so the same cleanup runs as File → Quit
    // (force-quit flag, floating-widget close, writeSettings, singleton deletes).
    ttek->slotQuit();
}

void Systray::slotPrepareMenu()
{
    if (ttek->isVisible() && !ttek->isMinimized()) {
        restoreAction->setEnabled(false);
        minimizeAction->setEnabled(true);
    } else if (ttek->isHidden() || ttek->isMinimized()) {
        restoreAction->setEnabled(true);
        minimizeAction->setEnabled(false);
    }
}

void Systray::newTag(QString name)
{
    qCDebug(FONTMATRIX_LOG) << "Systray::newTag" << name;
    if (tagActions.contains(name))
        return; // already added

    QAction *tmp = tagMenu->addAction(name);
    tmp->setCheckable(true);
    QList<FontItem *> taggedFonts = FMFontDb::DB()->Fonts(name, FMFontDb::Tags);
    int nActivated(0);
    int nFonts(taggedFonts.count());
    for (int i = 0; i < nFonts; ++i) {
        // 		qDebug()<<taggedFonts[i]->path();
        Q_ASSERT(taggedFonts[i]);
        if (taggedFonts[i]->isActivated()) {
            ++nActivated;
        }
    }
    if (nActivated < nFonts) {
        if (nActivated > 0) {
            QFont f(tmp->font());
            f.setBold(true);
            tmp->setFont(f);
            tmp->setText(name + " " + QString::number(nActivated) + "/" + QString::number(nFonts));
            tmp->setChecked(false);
        } else {
            tmp->setChecked(false);
        }
    } else
        tmp->setChecked(true);

    tagActions[name] = tmp;
}

void Systray::deleteTag(const QString &name)
{
    QAction *tmp = tagActions.value(name);
    if (tmp) {
        tagMenu->removeAction(tmp);
        tagActions.remove(name);
    }
}

void Systray::createActions()
{
    activateAllAction = new QAction(i18nc("@action:inmenu", "&Activate all"), this);
    connect(activateAllAction, &QAction::triggered, this, &Systray::slotActivateAll);

    deactivateAllAction = new QAction(i18nc("@action:inmenu", "&Deactivate all"), this);
    connect(deactivateAllAction, &QAction::triggered, this, &Systray::slotDeactivateAll);

    minimizeAction = new QAction(i18nc("@action:inmenu", "Mi&nimize"), this);
    connect(minimizeAction, &QAction::triggered, this, &Systray::slotMinimize);

    restoreAction = new QAction(i18nc("@action:inmenu", "&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &Systray::slotRestore);

    quitAction = new QAction(i18nc("@action:inmenu", "E&xit"), this);
    connect(quitAction, &QAction::triggered, this, &Systray::slotQuit);
}

void Systray::createTrayIcon()
{
    if (!ttek)
        ttek = typotek::getInstance();

    trayIconMenu = new QMenu(nullptr);
    trayIconMenu->addAction(activateAllAction);
    trayIconMenu->addAction(deactivateAllAction);
    tagMenu = trayIconMenu->addMenu(i18nc("@title:menu", "&Tags"));
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new KStatusNotifierItem(QStringLiteral("fontmatrix"), this);
    trayIcon->setCategory(KStatusNotifierItem::ApplicationStatus);
    trayIcon->setTitle(QStringLiteral("Fontmatrix"));
    trayIcon->setStandardActionsEnabled(false);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, &KStatusNotifierItem::activateRequested, this, [](bool /*active*/, const QPoint & /*pos*/) {
        if (!ttek)
            return;
        if (ttek->isVisible())
            ttek->hide();
        else
            ttek->show();
    });
}

void Systray::createTagMenu()
{
    if (!ttek)
        ttek = typotek::getInstance();

    QStringList tmp(FMFontDb::DB()->getTags());
    tmp.sort();
    for (const auto &tagName : std::as_const(tmp)) {
        // 		if (tagName != "Activated_On" && tagName != "Activated_Off")
        newTag(tagName);
    }

    connect(tagMenu, &QMenu::triggered, this, &Systray::slotTagMenuClicked);
}

bool Systray::isVisible()
{
    return trayIcon->status() != KStatusNotifierItem::Passive;
}

bool Systray::hasActivateAll()
{
    return activateAllAction->isVisible();
}

bool Systray::allConfirmation()
{
    return showAllConfirmation;
}

bool Systray::tagsConfirmation()
{
    return showTagsConfirmation;
}

void Systray::requireAllConfirmation(bool doRequire)
{
    showAllConfirmation = doRequire;
    FMConfig::setValue(QStringLiteral("Systray/AllConfirmation"), doRequire);
}

void Systray::requireTagsConfirmation(bool doRequire)
{
    showTagsConfirmation = doRequire;
    FMConfig::setValue(QStringLiteral("Systray/TagsConfirmation"), doRequire);
}

void Systray::updateTagMenu(const QStringList &nameOfFontWhichCausedThisUpdate)
{
    QStringList tags(tagActions.keys());
    bool lazy = true;
    for (const auto &tag : std::as_const(tags)) {
        QList<FontItem *> taggedFonts = FMFontDb::DB()->Fonts(tag, FMFontDb::Tags);
        //		ttek->resetFilter();
        for (auto *fit : std::as_const(taggedFonts)) {
            if (nameOfFontWhichCausedThisUpdate.contains(fit->path())) { // we’re concerned
                lazy = false;
                break;
            }
        }
    }
    if (lazy)
        return;
    for (const auto &tag : std::as_const(tags)) {
        deleteTag(tag);
    }

    if (!ttek)
        ttek = typotek::getInstance();

    QStringList tmp(FMFontDb::DB()->getTags());
    tmp.sort();
    for (const auto &tagName : std::as_const(tmp)) {
        // 		if (tagName != "Activated_On" && tagName != "Activated_Off")
        newTag(tagName);
    }
}

#include "moc_systray.cpp"
