//
// C++ Interface: fmaltselector
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMALTSELECTOR_H
#define FMALTSELECTOR_H

#include <QWidget>
#include <QVariant>
#include <QAbstractItemModel>
#include <QAbstractItemDelegate>

#include "fmaltcontext.h"

#include "ui_altselectorwidget.h"


class FMAltSelectorModel : public QAbstractItemModel
{
	Q_OBJECT

	class AltItem
	{
	public:
		enum Type{TEXT = 0, PARAGRAPH, WORD, CHUNK, GLYPH};

		AltItem(Type t, QVariant d)
			:T(t),data(d) {}
		~AltItem(){qDeleteAll(children);}
		const Type T;
		const QVariant data;
		AltItem * parent;
		QList<int> alts;

		void addChild(AltItem* ai)
		{
			ai->parent = this;
			children.append(ai);
		}
		AltItem* child(int row){return children.at(row);}
		int childCount(){return children.count();}
		int row()
		{
			if(parent)
				return parent->children.indexOf(const_cast<AltItem*>(this));
			return 0;
		}
		int columnCount()
		{
			if((T == TEXT) || (T == GLYPH))
				return 2;
			return 1;
		}


	private:
		QList<AltItem*> children;
	};

	AltItem * rootItem;

	// we need a delegate to display alt glyphs / selection widget
	class FMAltItemDelegate : public QAbstractItemDelegate
	{
		FMAltItemDelegate();
		const FMAltSelectorModel * pmodel;
	public:
		FMAltItemDelegate(FMAltSelectorModel* model);
		~FMAltItemDelegate(){delete pmodel;}

		void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
		QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	};
	FMAltItemDelegate * altDelegate;

public:
	FMAltSelectorModel();
	~FMAltSelectorModel();

	void reModel(FMAltContext * ctx);

	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	QModelIndex parent ( const QModelIndex & index ) const;
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

	FMAltItemDelegate * AltDelegate(){return altDelegate;}
	friend class FMAltItemDelegate;
};


// the container of the view
class FMAltSelector : public QWidget , private Ui::AltSelectorWidget
{
	Q_OBJECT

	FMAltSelectorModel * m_model;

public:
	FMAltSelector(QWidget * parent);
	~FMAltSelector(){}

public slots:
	void fillFromContext();
};

#endif // FMALTSELECTOR_H

