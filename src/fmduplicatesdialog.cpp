/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmduplicatesdialog.h"
#include "fmactivate.h"
#include "fmduplicates.h"
#include "fmfontdb.h"
#include "fmportal.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "mainviewwidget.h"
#include "typotek.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KSandbox>

#include <QApplication>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
enum Column {
    FileColumn = 0,
    FolderColumn,
    SizeColumn,
    VersionColumn,
    StatusColumn
};
constexpr int FontRole = Qt::UserRole + 1;
}

FMDuplicatesDialog::FMDuplicatesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Duplicates"));
    setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QVBoxLayout(this);

    auto *top = new QHBoxLayout;
    auto *kindLabel = new QLabel(i18nc("@label:listbox", "Show:"), this);
    m_kind = new QComboBox(this);
    m_kind->addItem(i18nc("@item:inlistbox files that are byte for byte the same", "Identical files"), int(FMDuplicates::IdenticalFiles));
    m_kind->addItem(i18nc("@item:inlistbox one family, style and version in files that differ", "Same font, other file"), int(FMDuplicates::SameFont));
    kindLabel->setBuddy(m_kind);
    top->addWidget(kindLabel);
    top->addWidget(m_kind);
    top->addStretch();
    layout->addLayout(top);

    m_summary = new QLabel(this);
    m_summary->setWordWrap(true);
    layout->addWidget(m_summary);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderLabels({i18nc("@title:column", "Font"),
                             i18nc("@title:column", "Folder"),
                             i18nc("@title:column", "Size"),
                             i18nc("@title:column", "Version"),
                             i18nc("@title:column", "Status")});
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setAlternatingRowColors(true);
    m_tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(m_tree);

    auto *actions = new QHBoxLayout;
    m_selectExtra = new QPushButton(i18nc("@action:button", "Tick the Extra Copies"), this);
    m_selectExtra->setToolTip(
        i18nc("@info:tooltip",
              "In every group of identical files, tick all the files but one: the one that is activated, or else the one in the shortest path"));
    m_showInList = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-find")), i18nc("@action:button", "Show in List"), this);
    m_openFolder = new QPushButton(QIcon::fromTheme(QStringLiteral("document-open-folder")), i18nc("@action:button", "Open Folder"), this);
    m_remove = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), i18nc("@action:button", "Remove from Collection"), this);
    m_remove->setToolTip(i18nc("@info:tooltip", "Forget the ticked files; they stay where they are on the disk"));
    m_trash = new QPushButton(QIcon::fromTheme(QStringLiteral("user-trash")), i18nc("@action:button", "Move to Trash"), this);
    m_trash->setToolTip(i18nc("@info:tooltip", "Move the ticked files to the trash and forget them"));
    actions->addWidget(m_selectExtra);
    actions->addWidget(m_showInList);
    actions->addWidget(m_openFolder);
    actions->addStretch();
    actions->addWidget(m_remove);
    actions->addWidget(m_trash);
    layout->addLayout(actions);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_kind, &QComboBox::currentIndexChanged, this, &FMDuplicatesDialog::fill);
    connect(m_tree, &QTreeWidget::itemChanged, this, &FMDuplicatesDialog::updateButtons);
    connect(m_tree, &QTreeWidget::currentItemChanged, this, &FMDuplicatesDialog::updateButtons);
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &FMDuplicatesDialog::showInList);
    connect(m_selectExtra, &QPushButton::clicked, this, &FMDuplicatesDialog::selectExtraCopies);
    connect(m_showInList, &QPushButton::clicked, this, &FMDuplicatesDialog::showInList);
    connect(m_openFolder, &QPushButton::clicked, this, &FMDuplicatesDialog::openFolder);
    connect(m_trash, &QPushButton::clicked, this, [this]() {
        act(Trash);
    });
    connect(m_remove, &QPushButton::clicked, this, [this]() {
        act(Remove);
    });

    resize(900, 560);
    fill();
}

FontItem *FMDuplicatesDialog::fontOf(const QTreeWidgetItem *item)
{
    return item ? static_cast<FontItem *>(item->data(FileColumn, FontRole).value<void *>()) : nullptr;
}

bool FMDuplicatesDialog::canTrash(FontItem *font)
{
    if (!font || font->isRemote())
        return false;
    const QFileInfo info(font->path());
    return info.isFile() && QFileInfo(info.absolutePath()).isWritable();
}

bool FMDuplicatesDialog::canRemove(FontItem *font)
{
    // a system font is read again at the next start, a remote one with its catalogue
    return font && !font->isRemote() && !typotek::getInstance()->isSysFont(font);
}

void FMDuplicatesDialog::fill()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const auto kind(static_cast<FMDuplicates::Kind>(m_kind->currentData().toInt()));
    const QList<QList<FontItem *>> groups(FMDuplicates::groups(kind));

    QList<FontItem *> all;
    for (const QList<FontItem *> &group : groups)
        all += group;
    QHash<FontItem *, QString> versions;
    if (!all.isEmpty()) {
        const QList<FontDBResult> rows(FMFontDb::DB()->getInfo(all, FMFontDb::VersionString, -1));
        for (const FontDBResult &row : rows) {
            if (row.first && !versions.contains(row.first))
                versions.insert(row.first, row.second.simplified());
        }
    }

    const QSignalBlocker blocker(m_tree);
    m_tree->clear();
    const QLocale locale;
    int files(0);
    for (const QList<FontItem *> &group : groups) {
        FontItem *first(group.first());
        auto *branch = new QTreeWidgetItem(m_tree);
        branch->setText(
            FileColumn,
            kind == FMDuplicates::IdenticalFiles
                ? i18ncp("@item a group of identical files, %1 how many, %2 the font", "%2 (%1 copy)", "%2 (%1 copies)", group.size(), first->fancyName())
                : i18ncp("@item a group of files of one font, %1 how many, %2 the font", "%2 (%1 file)", "%2 (%1 files)", group.size(), first->fancyName()));
        branch->setFirstColumnSpanned(true);
        QFont bold(branch->font(FileColumn));
        bold.setBold(true);
        branch->setFont(FileColumn, bold);
        branch->setFlags(Qt::ItemIsEnabled);
        for (FontItem *font : group) {
            auto *leaf = new QTreeWidgetItem(branch);
            const QFileInfo info(font->isRemote() ? font->localPath() : font->path());
            leaf->setText(FileColumn, font->isRemote() ? font->path() : info.fileName());
            leaf->setToolTip(FileColumn, font->path());
            leaf->setText(FolderColumn, font->isRemote() ? QString() : info.absolutePath());
            leaf->setToolTip(FolderColumn, info.absolutePath());
            leaf->setText(SizeColumn, info.exists() ? locale.formattedDataSize(info.size()) : QString());
            leaf->setText(VersionColumn, versions.value(font));
            QStringList status;
            if (typotek::getInstance()->isSysFont(font))
                status << i18nc("@item the font is one of the system", "System");
            else if (font->isActivated())
                status << i18nc("@item the font is activated", "Activated");
            if (font->isRemote())
                status << i18nc("@item the font is in a remote directory", "Remote");
            if (!canTrash(font) && !font->isRemote())
                status << i18nc("@item the folder of the file cannot be written to", "Read-only");
            leaf->setText(StatusColumn, status.join(QStringLiteral(", ")));
            leaf->setData(FileColumn, FontRole, QVariant::fromValue(static_cast<void *>(font)));
            if (canTrash(font) || canRemove(font)) {
                leaf->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
                leaf->setCheckState(FileColumn, Qt::Unchecked);
            } else {
                leaf->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                leaf->setToolTip(StatusColumn, i18nc("@info:tooltip", "This file cannot be removed from here: it would come back at the next start"));
            }
            ++files;
        }
    }
    m_tree->expandAll();

    if (groups.isEmpty())
        m_summary->setText(kind == FMDuplicates::IdenticalFiles ? i18nc("@info", "No file of the collection is there twice.")
                                                                : i18nc("@info", "No font of the collection is there in two different files."));
    else
        m_summary->setText(kind == FMDuplicates::IdenticalFiles
                               ? i18ncp("@info %1 how many groups, %2 how many files",
                                        "%2 files are byte for byte the same as each other. Any of them can go.",
                                        "%2 files, in %1 groups, are byte for byte the same as the others of their group. Any of them can go.",
                                        groups.size(),
                                        files)
                               : i18ncp("@info %1 how many groups, %2 how many files",
                                        "%2 files declare the same family, style and version, but differ. Which one to keep is your decision.",
                                        "%2 files, in %1 groups, declare the same family, style and version as the others of their group, but "
                                        "differ. Which one to keep is your decision.",
                                        groups.size(),
                                        files));
    QApplication::restoreOverrideCursor();
    updateButtons();
}

QList<QTreeWidgetItem *> FMDuplicatesDialog::checkedFiles() const
{
    QList<QTreeWidgetItem *> ret;
    for (int g(0); g < m_tree->topLevelItemCount(); ++g) {
        QTreeWidgetItem *branch(m_tree->topLevelItem(g));
        for (int f(0); f < branch->childCount(); ++f) {
            if (branch->child(f)->checkState(FileColumn) == Qt::Checked)
                ret << branch->child(f);
        }
    }
    return ret;
}

void FMDuplicatesDialog::updateButtons()
{
    const QList<QTreeWidgetItem *> checked(checkedFiles());
    bool trash(!checked.isEmpty());
    bool remove(!checked.isEmpty());
    for (QTreeWidgetItem *item : checked) {
        trash = trash && canTrash(fontOf(item));
        remove = remove && canRemove(fontOf(item));
    }
    m_trash->setEnabled(trash);
    m_remove->setEnabled(remove);
    m_selectExtra->setEnabled(m_kind->currentData().toInt() == int(FMDuplicates::IdenticalFiles) && m_tree->topLevelItemCount() > 0);
    FontItem *current(fontOf(m_tree->currentItem()));
    m_showInList->setEnabled(current);
    m_openFolder->setEnabled(current && !current->isRemote());
}

void FMDuplicatesDialog::selectExtraCopies()
{
    // the copy that stays: the one in use, else one that could not go anyway, else the shortest path
    auto rank = [](FontItem *font) {
        if (font->isActivated() && !typotek::getInstance()->isSysFont(font))
            return 0;
        if (!canTrash(font) && !canRemove(font))
            return 1;
        return 2;
    };
    const QSignalBlocker blocker(m_tree);
    for (int g(0); g < m_tree->topLevelItemCount(); ++g) {
        QTreeWidgetItem *branch(m_tree->topLevelItem(g));
        QTreeWidgetItem *keep(nullptr);
        for (int f(0); f < branch->childCount(); ++f) {
            QTreeWidgetItem *leaf(branch->child(f));
            FontItem *font(fontOf(leaf));
            if (!keep) {
                keep = leaf;
                continue;
            }
            FontItem *kept(fontOf(keep));
            if (rank(font) < rank(kept) || (rank(font) == rank(kept) && font->path().size() < kept->path().size()))
                keep = leaf;
        }
        for (int f(0); f < branch->childCount(); ++f) {
            QTreeWidgetItem *leaf(branch->child(f));
            if (leaf->flags() & Qt::ItemIsUserCheckable)
                leaf->setCheckState(FileColumn, leaf == keep ? Qt::Unchecked : Qt::Checked);
        }
    }
    updateButtons();
}

void FMDuplicatesDialog::showInList()
{
    FontItem *font(fontOf(m_tree->currentItem()));
    if (!font)
        return;
    if (!typotek::getInstance()->getTheMainView()->slotFontSelectedByName(font->path()))
        typotek::getInstance()->showStatusMessage(i18nc("@info:status", "This font is not in the list as it is filtered now"));
}

void FMDuplicatesDialog::openFolder()
{
    FontItem *font(fontOf(m_tree->currentItem()));
    if (font && !font->isRemote())
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(font->path()).absolutePath()));
}

bool FMDuplicatesDialog::moveToTrash(const QString &path)
{
    // inside a sandbox Qt would find a trash of the sandbox, not the one of the desktop
    if (KSandbox::isFlatpak())
        return FMPortal::trash(path);
    return QFile::moveToTrash(path);
}

void FMDuplicatesDialog::act(Action action)
{
    const QList<QTreeWidgetItem *> checked(checkedFiles());
    if (checked.isEmpty())
        return;

    // one file of every group stays
    for (int g(0); g < m_tree->topLevelItemCount(); ++g) {
        QTreeWidgetItem *branch(m_tree->topLevelItem(g));
        int left(0);
        for (int f(0); f < branch->childCount(); ++f) {
            if (branch->child(f)->checkState(FileColumn) != Qt::Checked)
                ++left;
        }
        if (left == 0) {
            KMessageBox::error(this,
                               xi18nc("@info",
                                      "Every file of <emphasis>%1</emphasis> is ticked. Leave at least one, or the font is gone.",
                                      fontOf(branch->child(0))->fancyName()),
                               i18nc("@title:window", "Duplicates"));
            m_tree->setCurrentItem(branch);
            return;
        }
    }

    QStringList paths;
    bool activated(false);
    for (QTreeWidgetItem *item : checked) {
        FontItem *font(fontOf(item));
        paths << font->path();
        activated = activated || (font->isActivated() && !typotek::getInstance()->isSysFont(font));
    }
    QString question(action == Trash ? i18ncp("@info", "Move this file to the trash?", "Move these %1 files to the trash?", paths.size())
                                     : i18ncp("@info",
                                              "Remove this file from the collection? It stays where it is on the disk.",
                                              "Remove these %1 files from the collection? They stay where they are on the disk.",
                                              paths.size()));
    if (activated)
        question += QLatin1Char(' ') + i18nc("@info", "The activated fonts among them are deactivated first.");
    if (KMessageBox::warningContinueCancelList(this,
                                               question,
                                               paths,
                                               i18nc("@title:window", "Duplicates"),
                                               action == Trash ? KGuiItem(i18nc("@action:button", "Move to Trash"), QStringLiteral("user-trash"))
                                                               : KStandardGuiItem::remove(),
                                               KStandardGuiItem::cancel(),
                                               QString(),
                                               KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous))
        != KMessageBox::Continue)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    FMFontDb *db(FMFontDb::DB());
    QStringList failed;
    int done(0);
    for (QTreeWidgetItem *item : checked) {
        FontItem *font(fontOf(item));
        if (font->isActivated() && !typotek::getInstance()->isSysFont(font))
            FMActivate::getInstance()->activate(QList<FontItem *>() << font, false);
        if (action == Trash && !moveToTrash(font->path())) {
            failed << font->path();
            continue;
        }
        font->deRenderAll();
        db->removeFilteredFont(font);
        if (!db->Remove(font->path())) {
            failed << font->path();
            continue;
        }
        qCDebug(FONTMATRIX_LOG) << (action == Trash ? "trashed" : "removed") << font->path();
        ++done;
    }
    QApplication::restoreOverrideCursor();

    // the sidebar, the list and this window all count again
    typotek::getInstance()->getTheMainView()->slotFontDbChanged();
    fill();
    typotek::getInstance()->showStatusMessage(
        action == Trash ? i18ncp("@info:status", "%1 file moved to the trash", "%1 files moved to the trash", done)
                        : i18ncp("@info:status", "%1 file removed from the collection", "%1 files removed from the collection", done));
    if (!failed.isEmpty())
        KMessageBox::errorList(this,
                               action == Trash ? i18nc("@info", "These files could not be moved to the trash:")
                                               : i18nc("@info", "These files could not be removed from the collection:"),
                               failed,
                               i18nc("@title:window", "Duplicates"));
}

#include "moc_fmduplicatesdialog.cpp"
