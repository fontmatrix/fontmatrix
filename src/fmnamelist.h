/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMNAMELIST_H
#define FMNAMELIST_H

#include <QElapsedTimer>
#include <QString>
#include <QTreeWidget>

class QTreeViewItem;

class FMNameList : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FMNameList(QWidget *parent);
    ~FMNameList() override;

    void slotNextFamily();
    void slotPreviousFamily();
    void slotNextFont();
    void slotPreviousFont();
    bool slotSetCurrent(const QString &fname);

Q_SIGNALS:
    void currentChanged(QTreeWidgetItem *, int);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    // 	private:
    // 		QString curString;
private:
    bool findAbove(QTreeWidgetItem *current, const QString &role);
    bool findBelow(QTreeWidgetItem *current, const QString &role);

    QString m_keyString;
    QElapsedTimer m_keyTime;
    const int m_waitKey;
};

#endif
