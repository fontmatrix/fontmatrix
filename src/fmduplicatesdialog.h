/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMDUPLICATESDIALOG_H
#define FMDUPLICATESDIALOG_H

#include <QDialog>
#include <QList>

class FontItem;
class QComboBox;
class QLabel;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * The fonts the collection holds more than once, group by group, and a way to
 * get rid of the extra files.
 *
 * The filter of the sidebar gives the same fonts as one flat list; here each
 * group is a branch, with what tells its files apart: where they are, their
 * size, their version and whether they are activated. The files ticked can be
 * moved to the trash (the file and its entry both go) or only removed from the
 * collection (the file stays where it is). One file of every group always
 * stays, and an activated font is deactivated before it goes.
 *
 * A system font comes back into the collection at the next start as long as its
 * file is there, so it can be trashed, where its folder allows it, but not only
 * removed. A font of a remote directory is neither: its catalogue brings it back.
 */
class FMDuplicatesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FMDuplicatesDialog(QWidget *parent = nullptr);
    ~FMDuplicatesDialog() override = default;

private:
    enum Action {
        Trash,
        Remove
    };

    void fill();
    void updateButtons();
    void selectExtraCopies();
    void showInList();
    void openFolder();
    void act(Action action);

    [[nodiscard]] QList<QTreeWidgetItem *> checkedFiles() const;
    [[nodiscard]] static FontItem *fontOf(const QTreeWidgetItem *item);
    [[nodiscard]] static bool canTrash(FontItem *font);
    [[nodiscard]] static bool canRemove(FontItem *font);
    static bool moveToTrash(const QString &path);

    QComboBox *m_kind = nullptr;
    QLabel *m_summary = nullptr;
    QTreeWidget *m_tree = nullptr;
    QPushButton *m_selectExtra = nullptr;
    QPushButton *m_showInList = nullptr;
    QPushButton *m_openFolder = nullptr;
    QPushButton *m_trash = nullptr;
    QPushButton *m_remove = nullptr;
};

#endif
