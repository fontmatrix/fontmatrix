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

#ifndef METAWIDGET_H
#define METAWIDGET_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QStringList>

#include "fmfontstrings.h"

class QStringListModel;
class QPushButton;
class QLineEdit;
class QComboBox;
class QHBoxLayout;

namespace Ui {
    class MetaWidget;
}

class MetaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetaWidget(QWidget *parent = 0);
    ~MetaWidget();

    int resultField;
    QString resultText;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MetaWidget *ui;

    static QStringListModel *mModel;
    static QStringList mList;
    QWidget *filterWidget;
    QComboBox *filterCombo;
    QLineEdit *filterLine;
    QPushButton *filterButton;
    QMap<QLineEdit*, FMFontDb::InfoItem> metFields;

signals:
    void filterAdded();

private slots:
    void addFilter();
};

#endif // METAWIDGET_H
