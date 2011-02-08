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
#include <QAbstractListModel>
#include <QListView>
#include <QMenu>
#include <QStringListModel>
#include <QMouseEvent>
#include <QHBoxLayout>


class FiltersDialogItem;
class FilterItem;
class FilterData;


class TagListModel : public QAbstractListModel
{
	Q_OBJECT
	const int specialTagsCount;

	QStringList currentTags;
public:
	enum TagListRole
	{
		TagType = Qt::UserRole,
		TagString
	};

	TagListModel(QObject * parent);
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	Qt::ItemFlags flags ( const QModelIndex & index ) const;

	void clearCurrents();
	void addToCurrents(const QString& t);
	void removeFromCurrents(const QString& t);

public slots:
	void tagsDBChanged();

};


class TagListView : public QListView
{
	Q_OBJECT

	int m_andOrKey;
public:
	TagListView(QWidget * parent):
			QListView(parent),
			m_andOrKey(0)
	{}

	int getAndKey(){int ret(m_andOrKey), m_andOrKey = 0; return ret;}

protected:
	void mouseReleaseEvent(QMouseEvent *event)
	{
		if(event->modifiers().testFlag(Qt::ShiftModifier))
			m_andOrKey = 1;
		else if(event->modifiers().testFlag(Qt::ControlModifier))
			m_andOrKey = 2;
		else
			m_andOrKey = 0;
		QListView::mouseReleaseEvent(event);
	}
};


namespace Ui {
    class FilterBar;
}

class FilterBar : public QWidget
{
    Q_OBJECT

public:
    explicit FilterBar(QWidget *parent = 0);
    ~FilterBar();

    void setFilterListLayout(QHBoxLayout *l){filterListLayout = l;}
    void setCurFilterWidget(QWidget * w){curFilterWidget = w;}

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FilterBar *ui;
    QHBoxLayout * filterListLayout;
    QWidget * curFilterWidget;

    QList<FilterItem*> filters;
    void addFilterItem(FilterData* f, bool process = true);
    void removeAllFilters();
    TagListModel * tagListModel;
//    QMenu * metaFieldsMenu;
    int metaFieldKey;

    QString filterString(FilterData *d, bool first = false);
    void loadFilters();
    QList<FiltersDialogItem*> items;
    static QString andOpString;
    static QString notOpString;
    static QString orOpString;

    QStringListModel *mModel;
    QStringList mList;

signals:
    void initSearch(int, QString);
    void filterChanged();

private slots:
    void processFilters();
    void slotPanoFilter();
    void metaFilter();
    void metaSelectField(int idx);

    void filtersDialog();

    void slotLoadFilter(const QString& fname);
    void slotRemoveFilter(const QString& fname);

    void slotRemoveFilterItem(bool process = true);

    void slotTagSelect(const QModelIndex & index);
    void slotTagEdit(const QModelIndex & index);
    void slotClearFilter();

    void slotToggleTags(bool t);
    void slotToggleMeta(bool t);
    void slotTogglePano(bool t);
    void slotToggleFilter(bool t);

public slots:
    void slotSaveFilter();

};

#endif // FILTERBAR_H
