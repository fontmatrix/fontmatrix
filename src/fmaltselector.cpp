//
// C++ Implementation: fmaltselector
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmaltselector.h"
#include "typotek.h"
#include "fontitem.h"
#include "fmfontdb.h"

#include <QPainter>
#include <QDebug>

FMAltSelectorModel::FMAltSelectorModel()
		:QAbstractItemModel()
{
	altDelegate = new FMAltItemDelegate(this);
	rootItem = 0;
}

FMAltSelectorModel::~FMAltSelectorModel()
{
	delete altDelegate;
	if(rootItem)
		delete rootItem;
}

void FMAltSelectorModel::reModel(FMAltContext * ctx)
{
	beginResetModel();
	if(rootItem)
		delete rootItem;
	rootItem = new AltItem(AltItem::TEXT,0);
	ctx->saveRun();
	for(int p(0); p < ctx->maxPar(); ++p)
	{
		ctx->setPar(p);
		AltItem * parItem = new AltItem(AltItem::PARAGRAPH, p);
		rootItem->addChild(parItem);
		for(int w(0); w < ctx->maxWord(); ++w)
		{
			ctx->setWord(w);
			if(ctx->wordString().isEmpty())
				continue;
			AltItem * wordItem = new AltItem(AltItem::WORD, ctx->wordString());
			parItem->addChild(wordItem);
			for(int c(0);c < ctx->maxChunk();++c)
			{
				ctx->setChunk(c);
				AltItem * chunkItem = new AltItem(AltItem::CHUNK, ctx->chunkString());
				wordItem->addChild(chunkItem);
				for(int g(0); g < ctx->maxGlyph(); ++g)
				{
					if( ctx->alts(g).count())
					{
						AltItem * glyphItem = new AltItem(AltItem::GLYPH, g);
						glyphItem->alts = ctx->alts(g);
						chunkItem->addChild(glyphItem);
					}
				}
			}
		}
	}
	ctx->restoreRun();

	endResetModel();
}

QModelIndex FMAltSelectorModel::index ( int row, int column, const QModelIndex & parent  ) const
{
	if(!rootItem)
		return QModelIndex();
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	AltItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<AltItem*>(parent.internalPointer());

	AltItem *childItem = parentItem->child(row);
	if (childItem)
	{
		if(column>0)
			qDebug()<<"C"<<column<< childItem->T;
//		qDebug()<<"Create Index"<<row<< column<< childItem->T;
		return createIndex(row, column, childItem);
	}
	else
		return QModelIndex();

}

QModelIndex FMAltSelectorModel::parent ( const QModelIndex & index ) const
{

	if(!rootItem)
		return QModelIndex();
	if (!index.isValid())
		return QModelIndex();

	AltItem *childItem = static_cast<AltItem*>(index.internalPointer());
	AltItem *parentItem = childItem->parent;

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);

}

int FMAltSelectorModel::rowCount(const QModelIndex & parent) const
{
	if(!rootItem)
		return 0;
	AltItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<AltItem*>(parent.internalPointer());

	return parentItem->childCount();

}

int FMAltSelectorModel::columnCount ( const QModelIndex & parent  ) const
{
	if(!rootItem)
		return 0;

	if (parent.isValid())
	{
		AltItem* pItem =  static_cast<AltItem*>(parent.internalPointer());
		if(pItem)
		{
			if(pItem->T == AltItem::CHUNK) // means the current index is a GLYPH
				return 2;
		}
		return 1;
	}
	else
		return 2;

}

Qt::ItemFlags FMAltSelectorModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

}

QVariant FMAltSelectorModel::data ( const QModelIndex & index, int role ) const
{
//	qDebug()<<"FMAltSelectorModel::data"<<index<<role;
	if(!index.isValid())
		return QVariant();

	if(index.column() == 0 && role == Qt::DisplayRole)
	{
		AltItem *item = static_cast<AltItem*>(index.internalPointer());
		if(item)
		{
			if(item->T == AltItem::PARAGRAPH)
			{
				return QString::fromLocal8Bit("¶") + QString::number(item->data.toInt());
			}
			else if(item->T == AltItem::WORD)
			{
				return item->data.toString();
			}
			else if(item->T == AltItem::CHUNK)
			{
				return item->data.toString();
			}
			else if(item->T == AltItem::GLYPH)
			{
				return QString("#") + QString::number(item->data.toInt());
			}
		}
		else
			qDebug()<<"Oops internal pointer is null";
	}

	return QVariant();
}

FMAltSelectorModel::FMAltItemDelegate::FMAltItemDelegate(FMAltSelectorModel* model)
		:QAbstractItemDelegate(), pmodel(model)
{
}

void FMAltSelectorModel::FMAltItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index)const
{
//	qDebug()<<"Paint"<<index.row()<<index.column() ;
	if(!index.isValid())
		return;
	FMAltSelectorModel::AltItem * item = static_cast<FMAltSelectorModel::AltItem*>(index.internalPointer());
	if((!item) )
	{
		qDebug()<<"Item is not"<< index.row()<<index.column();
		return;
	}
	if( (item->T != FMAltSelectorModel::AltItem::GLYPH))
	{
		qDebug()<<"Item is not a GLYPH item"<<item->T<<index.row()<<index.column();

		return;
	}
	FontItem * fi(typotek::getInstance()->getSelectedFont());
	if(!fi)
	{
		qDebug()<<"Unable to get the selected font";
		return;
	}
	qDebug()<<"Paint"<<index.row()<<index.column()<< item->alts;
	double fsize(22.0);
	painter->save();
	foreach(int idx, item->alts)
	{
		QImage img(fi->glyphImage(idx, fsize));
		int ssize(img.width());
		QRect targetRect(img.rect());
		targetRect.moveTo(option.rect.x(), option.rect.y());
		painter->drawImage(targetRect, img, img.rect());
		painter->translate(ssize + 3, 0);
	}
	painter->restore();
}

QSize FMAltSelectorModel::FMAltItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QSize ret;

	double fsize(22.0);
	FontItem * fi(typotek::getInstance()->getSelectedFont());
	if(!fi)
		return ret;
	FMAltSelectorModel::AltItem * item = static_cast<FMAltSelectorModel::AltItem*>(index.internalPointer());
	if(!item)
		return ret;
	foreach(int idx, item->alts)
	{
		QImage img(fi->glyphImage(idx, fsize));
		ret.rwidth() += img.width() + 3;
		ret.rheight() = qMax(ret.height(), img.height());
	}
	return ret;
}


FMAltSelector::FMAltSelector(QWidget * parent)
		:QWidget(parent)
{
	setupUi ( this );
	m_model = new FMAltSelectorModel();
	altSelectView->setModel(m_model);
	altSelectView->setItemDelegateForColumn(1, m_model->AltDelegate());

	FMAltContextLib::GetConnected(this, SLOT(fillFromContext()));

	// disable it
	setVisible(false);

}

void FMAltSelector::fillFromContext()
{
	qDebug("FMAltSelector::fillFromContext");
	m_model->reModel( FMAltContextLib::GetCurrentContext() );

}
