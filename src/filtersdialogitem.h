/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef FILTERSDIALOGITEM_H
#define FILTERSDIALOGITEM_H

#include <QWidget>
#include <QString>

namespace Ui {
    class FiltersDialogItem;
}

class FiltersDialogItem : public QWidget
{
    Q_OBJECT

public:
    explicit FiltersDialogItem(const QString& name, const QString& f, QWidget *parent = 0);
    ~FiltersDialogItem();

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

private:
    Ui::FiltersDialogItem *ui;

    void setButtonsVisible(bool v);
    QString filterName;

private slots:
    void slotFilter();
    void slotRemove();

signals:
    void Filter(QString);
    void Remove(QString);
};

#endif // FILTERSDIALOGITEM_H
