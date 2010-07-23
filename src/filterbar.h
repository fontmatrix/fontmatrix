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

#ifndef FILTERBAR_H
#define FILTERBAR_H

#include <QWidget>
#include <QList>
#include <QMap>

class FilterItem;
class FilterData;

namespace Ui {
    class FilterBar;
}

class FilterBar : public QWidget
{
    Q_OBJECT

public:
    explicit FilterBar(QWidget *parent = 0);
    ~FilterBar();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FilterBar *ui;

    QList<FilterItem*> filters;
    void addFilter(FilterData*);
    void removeAllFilters();

signals:
    void initSearch(int, QString);
    void filterChanged();

private slots:
    void processFilters();
    void slotPanoFilter();
    void loadTags();
    void panoseDialog();
    void metaDialog();

    void filtersDialog();

    void slotSaveFilter(const QString& fname);
    void slotLoadFilter(const QString& fname);

    void slotRemoveFilter(bool process = true);

    void slotTagSelect(const QString& t);
    void slotClearFilter();
};

#endif // FILTERBAR_H
