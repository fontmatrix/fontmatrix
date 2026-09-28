/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmrepair.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "mainviewwidget.h"
#include "typotek.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

FmRepair::FmRepair(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    // 	listItems.clear();
    fillLists();
    doConnect();
}

FmRepair::~FmRepair()
{
    // 	for (auto* lit : listItems)
    // 	{
    // 		if(lit)
    // 			delete lit;
    // 	}
}

void FmRepair::doConnect()
{
    connect(closeButton, &QPushButton::clicked, this, &FmRepair::close);

    connect(selectAllDead, &QPushButton::clicked, this, &FmRepair::slotSelAllDead);
    connect(removeDead, &QPushButton::clicked, this, &FmRepair::slotRemoveDead);

    connect(selectAllActNot, &QPushButton::clicked, this, &FmRepair::slotSelAllActNotLinked);
    connect(relinkActNot, &QPushButton::clicked, this, &FmRepair::slotRelinkActNotLinked);
    connect(deactActNot, &QPushButton::clicked, this, &FmRepair::slotDeactivateActNotLinked);

    connect(selectAllDeactLink, &QPushButton::clicked, this, &FmRepair::slotSelAllDeactLinked);
    connect(delinkDeactLink, &QPushButton::clicked, this, &FmRepair::slotDelinkDeactLinked);
    connect(activateDeactLink, &QPushButton::clicked, this, &FmRepair::slotActivateDeactLinked);

    connect(selectAllUnreferenced, &QPushButton::clicked, this, &FmRepair::slotSelectAllUnref);
    connect(removeUnreferenced, &QPushButton::clicked, this, &FmRepair::slotRemoveUnref);
}

void FmRepair::fillLists()
{
    fillDeadLink();
    fillActNotLinked();
    fillDeactLinked();
    fillUnreferenced();
}

void FmRepair::fillDeadLink()
{
    typotek *t = typotek::getInstance();
    deadList->clear();
    QDir md(t->getManagedDir());
    md.setFilter(QDir::Files);
    QFileInfoList list = md.entryInfoList();
    for (int i(0); i < list.count(); ++i) {
        if (list[i].isSymLink()) {
            if (!QFileInfo::exists(list[i].symLinkTarget())) {
                auto lit = new QListWidgetItem(list[i].absoluteFilePath());
                lit->setCheckState(Qt::Unchecked);
                lit->setToolTip(list[i].absoluteFilePath());
                deadList->addItem(lit);
                // 				listItems << lit;
            }
        }
    }
}

void FmRepair::fillActNotLinked()
{
    typotek *t = typotek::getInstance();
    actNotLinkList->clear();
    QList<FontItem *> flist(FMFontDb::DB()->AllFonts());
    QStringList activated;
    for (int i(0); i < flist.count(); ++i) {
        if ((!t->isSysFont(flist[i])) && (flist[i]->isActivated()))
            activated << flist[i]->path();
    }

    QStringList linked;
    QDir md(t->getManagedDir());
    md.setFilter(QDir::Files);
    QFileInfoList list = md.entryInfoList();
    for (int i(0); i < list.count(); ++i) {
        if (list[i].isSymLink()) {
            if (QFileInfo::exists(list[i].symLinkTarget())) {
                // 				qDebug()<< "ACT NOT LINK "<<list[i].symLinkTarget();
                linked << list[i].symLinkTarget();
            } else {
                qCDebug(FONTMATRIX_LOG) << list[i].filePath() << " is a broken symlink";
            }
        } else {
            qCDebug(FONTMATRIX_LOG) << list[i].filePath() << " is not a symlink";
        }
    }

    for (int i(0); i < activated.count(); ++i) {
        if (!linked.contains(activated[i])) {
            auto lit = new QListWidgetItem(activated[i]);
            lit->setCheckState(Qt::Unchecked);
            lit->setToolTip(activated[i]);
            actNotLinkList->addItem(lit);
            // 			listItems << lit;
        }
    }
}

void FmRepair::fillDeactLinked()
{
    typotek *t = typotek::getInstance();
    deactLinkList->clear();
    QList<FontItem *> flist(FMFontDb::DB()->AllFonts());
    QStringList deactivated;
    for (int i(0); i < flist.count(); ++i) {
        if (!t->isSysFont(flist[i]) && !flist[i]->isRemote() && !flist[i]->isActivated())
            deactivated << flist[i]->path();
    }
    // 	qDebug() << deactivated.join("\nDEACT : ");
    QStringList linked;
    QDir md(t->getManagedDir());
    md.setFilter(QDir::Files);
    QFileInfoList list = md.entryInfoList();
    for (int i(0); i < list.count(); ++i) {
        if (list[i].isSymLink()) {
            if (QFileInfo::exists(list[i].symLinkTarget())) {
                linked << list[i].symLinkTarget();
            }
        }
    }

    for (int i(0); i < linked.count(); ++i) {
        if (deactivated.contains(linked[i])) {
            // 			qDebug() << "NO " << linked[i] ;
            auto lit = new QListWidgetItem(linked[i]);
            lit->setCheckState(Qt::Unchecked);
            lit->setToolTip(linked[i]);
            deactLinkList->addItem(lit);
            // 			listItems << lit;
        } else {
            // 			qDebug() << "OK " << linked[i] ;
        }
    }
}

void FmRepair::slotSelAllDead()
{
    for (int i(0); i < deadList->count(); ++i) {
        deadList->item(i)->setCheckState(Qt::Checked);
    }
}

void FmRepair::slotRemoveDead()
{
    for (int i(0); i < deadList->count(); ++i) {
        if (deadList->item(i)->checkState() == Qt::Checked) {
            QFile f(deadList->item(i)->text());
            f.remove();
        }
    }
    fillDeadLink();
}

void FmRepair::slotSelAllActNotLinked()
{
    for (int i(0); i < actNotLinkList->count(); ++i) {
        actNotLinkList->item(i)->setCheckState(Qt::Checked);
    }
}

void FmRepair::slotRelinkActNotLinked()
{
    typotek *t = typotek::getInstance();
    for (int i(0); i < actNotLinkList->count(); ++i) {
        if (actNotLinkList->item(i)->checkState() == Qt::Checked) {
            FontItem *font = FMFontDb::DB()->Font(actNotLinkList->item(i)->text());
            if (font) {
                QFile f(font->path());
                f.link(t->getManagedDir() + QDir::separator() + font->activationName());

                if (!font->afm().isEmpty()) {
                    QFile af(font->afm());
                    af.link(t->getManagedDir() + QDir::separator() + font->activationAFMName());
                }
            }
        }
    }
    fillActNotLinked();
}

void FmRepair::slotDeactivateActNotLinked()
{
    for (int i(0); i < actNotLinkList->count(); ++i) {
        if (actNotLinkList->item(i)->checkState() == Qt::Checked) {
            FontItem *font = FMFontDb::DB()->Font(actNotLinkList->item(i)->text());
            if (font) {
                font->setActivated(false);
            }
        }
    }

    fillActNotLinked();
}

void FmRepair::slotSelAllDeactLinked()
{
    for (int i(0); i < deactLinkList->count(); ++i) {
        deactLinkList->item(i)->setCheckState(Qt::Checked);
    }
}

void FmRepair::slotDelinkDeactLinked()
{
    typotek *t = typotek::getInstance();
    for (int i(0); i < deactLinkList->count(); ++i) {
        if (deactLinkList->item(i)->checkState() == Qt::Checked) {
            FontItem *font = FMFontDb::DB()->Font(deactLinkList->item(i)->text());
            if (font) {
                QFile f(t->getManagedDir() + QDir::separator() + font->activationName());
                f.remove();
            }
        }
    }
    fillDeactLinked();
}

void FmRepair::slotActivateDeactLinked()
{
    for (int i(0); i < deactLinkList->count(); ++i) {
        if (deactLinkList->item(i)->checkState() == Qt::Checked) {
            FontItem *font = FMFontDb::DB()->Font(deactLinkList->item(i)->text());
            if (font) {
                font->setActivated(true);
            }
        }
    }
    fillDeactLinked();
}

void FmRepair::fillUnreferenced()
{
    unrefList->clear();
    for (const auto allFontNames = FMFontDb::DB()->AllFontNames(); const auto &fid : allFontNames) {
        if (!QFile::exists(fid)) {
            auto lit = new QListWidgetItem(fid);
            lit->setCheckState(Qt::Unchecked);
            unrefList->addItem(lit);
            // 			listItems << lit;
        }
    }
}

void FmRepair::slotSelectAllUnref()
{
    for (int i(0); i < unrefList->count(); ++i) {
        unrefList->item(i)->setCheckState(Qt::Checked);
    }
}

void FmRepair::slotRemoveUnref()
{
    FMFontDb *db(FMFontDb::DB());
    QStringList failed;
    QList<FontItem *> flist(typotek::getInstance()->getTheMainView()->curFonts());
    for (int i(0); i < unrefList->count(); ++i) {
        if (unrefList->item(i)->checkState() == Qt::Checked) {
            FontItem *curItem = nullptr;
            QString fId(unrefList->item(i)->text());
            for (auto *it : std::as_const(flist)) {
                if (it->path() == fId) {
                    curItem = it;
                    break;
                }
            }
            if (!db->Remove(fId))
                failed << fId;
            else if (curItem)
                flist.removeAll(curItem);
        }
    }
    typotek::getInstance()->getTheMainView()->setCurFonts(flist);
    // 	if(failed.count() > 0)
    // 	{
    // 		QMessageBox::warning(this,"Fontmatrix - warning",failed.join("\n"));
    // 	}
    fillUnreferenced();
}

#include "moc_fmrepair.cpp"
