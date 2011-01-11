// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TAGSWIDGET_LISTMODEL_H
#define TAGSWIDGET_LISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QStringList>
#include <QVariant>

class FontItem;

class TagsWidget_ListModel : public  QAbstractListModel
{
	Q_OBJECT

	QList<FontItem*> fonts;
	QStringList tags;

public:

	TagsWidget_ListModel(QObject * parent);
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	Qt::ItemFlags flags ( const QModelIndex & index ) const;

	void setFonts(const QList<FontItem*>& flist);

};

#endif // TAGSWIDGET_LISTMODEL_H
