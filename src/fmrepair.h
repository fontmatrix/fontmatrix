/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMREPAIR_H
#define FMREPAIR_H

#include <QDialog>
#include <ui_repair.h>

class FmRepair : public QDialog, private Ui::repairDialog
{
    Q_OBJECT
public:
    explicit FmRepair(QWidget *parent);
    ~FmRepair() override;

private:
    void fillDeadLink();
    void fillActNotLinked();
    void fillDeactLinked();
    void fillUnreferenced();
    void fillLists();

    void doConnect();

    // 		QList<QListWidgetItem*> listItems;

private Q_SLOTS:
    void slotSelAllDead();
    void slotRemoveDead();

    void slotSelAllActNotLinked();
    void slotRelinkActNotLinked();
    void slotDeactivateActNotLinked();

    void slotSelAllDeactLinked();
    void slotDelinkDeactLinked();
    void slotActivateDeactLinked();

    void slotSelectAllUnref();
    void slotRemoveUnref();
};

#endif
