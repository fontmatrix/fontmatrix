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

#ifndef PROGRESSBARDUO_H
#define PROGRESSBARDUO_H

#include <QDialog>

namespace Ui {
    class ProgressBarDuo;
}

class ProgressBarDuo : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressBarDuo(QWidget *parent = 0);
    ~ProgressBarDuo();

    void setLabel(const QString& s, int n);
    void setValue(int value, int n);
    void setMax(int max, int n);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ProgressBarDuo *ui;

signals:
    void Canceled();
};

#endif // PROGRESSBARDUO_H
