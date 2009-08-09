/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
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
#ifndef FMPREVIEWLIST_H
#define FMPREVIEWLIST_H


#include <QListView>
#include <QAbstractListModel>
#include <QIconEngineV2>
#include <QPixmap>
#include <QPoint>

class FontItem;
class MainViewWidget;
class QListView;


// Rather than fighting against Qt to not resize our icons, draw them ourselves.
class FMPreviewIconEngine : public QIconEngineV2
{
public:
	FMPreviewIconEngine();
	~FMPreviewIconEngine();
	void paint ( QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state );
	void addPixmap ( const QPixmap & pixmap, QIcon::Mode mode, QIcon::State state );

private:
	QPixmap m_p;
	static QVector<QRgb> m_selPalette;
	QVector<QRgb> actualSelPalette(const QVector<QRgb>& orig);

};


class FMPreviewView : public QListView
{
	Q_OBJECT
public:
	FMPreviewView(QWidget * parent = 0);
	~FMPreviewView(){}
	int getUsedWidth() const{return usedWidth;}
protected:
	void resizeEvent ( QResizeEvent * event );
	QPoint startDragPoint;
	bool dragFlag;
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private:
	int usedWidth;

public slots:
	void updateLayout();
	void setCurrentFont(const QString& name);

signals:
	void widthChanged(int);

};

class FMPreviewModel : public QAbstractListModel
{
public:
	enum PreviewItemRole{
		PathRole = Qt::UserRole + 1
		   };

	FMPreviewModel ( QObject * pa , FMPreviewView * wPa);
	//returns a preview
	QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
	//returns flags for items
	Qt::ItemFlags flags ( const QModelIndex &index ) const;
	//returns the number of items
	int rowCount ( const QModelIndex &parent ) const;

	void dataChanged();


private:
	FMPreviewView *m_view;

	QString styleTooltipName;
	QString styleTooltipPath;
};



#endif
