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
#ifndef MAINVIEWWIDGET_H
#define MAINVIEWWIDGET_H

#include <ui_mainview.h>

#include <QStringList>
#include <QIcon>



class QGraphicsScene;
class typotek;
class FontItem;
class QTextEdit;
class QGridLayout;
class QTreeWidgetItem;
class QGraphicsRectItem;
class QButtonGroup;
class QWebView;
class ListDockWidget;
struct OTFSet;
class FMLayout;
class FMPreviewModel;

/**
MainViewWidget inherits from an ui designed.

	@author Pierre Marchand <pierre@oep-h.com>
*/
class MainViewWidget :  public QWidget, private Ui::MainView
{
		Q_OBJECT

	public:
		MainViewWidget ( QWidget *parent );

		~MainViewWidget();
	private:
		QStringList ord;
		QStringList fields;
		typotek *typo;
		ListDockWidget *m_lists;
		QString faceIndex;
		QString lastIndex;
//		QList<FontItem*> currentFonts; *moved to FMFontDB*
		QList<FontItem*> orderedCurrentFonts;
		QString sampleText;
		QGridLayout *tagLayout;
		QString currentOrdering;
		FontItem *theVeryFont; 
		bool fontsetHasChanged;
		bool activateByFamilyOnly;
		bool m_forceReloadSelection;

		void doConnect();
		void disConnect();
		void allActivation(bool act);
		void activation(QList<FontItem*> fit, bool act);
		void fillTree();
		void updateTree(bool checkFontActive = false);
		
		
		void operateFilter(QList<FontItem*> allFiltered, const QString filterName);
		
		QStringList openKeys;
		QString curItemName;
		

		
// 		QMap<QString, QPair<int,int> > uniPlanes;
// 		void fillUniPlanes();
		
//		void fillOTTree();
//		OTFSet deFillOTTree();
		
		bool renderingLock;

		
		
// 		void prepare(QList<FontItem*> fonts);
// 		QList<FontItem*> theTaggedFonts;		
// 		bool contextMenuReq;
// 		QPoint contextMenuPos;
		
		QIcon iconPS1;
		QIcon iconTTF;
		QIcon iconOTF;
		
		QString currentDownload;
		
//		QUrl infoCSSUrl;

		
		FMPreviewModel * previewModel;

	public slots:
		void slotOrderingChanged ( QString s );
		void slotFontSelected ( QTreeWidgetItem * item, int column );
		bool slotFontSelectedByName(const QString& fname);
//		void slotInfoFont();
//		void slotView(bool needDeRendering = false);
		void slotSearch(int, QString);
		void slotShowFamily(const QModelIndex& familyIdx);
		void slotQuitFamily();
		
//		void slotFontAction(QTreeWidgetItem * item, int column );
//                bool slotFontActionByName(const QString &fname);
//                bool slotFontActionByNames(QStringList fnames);
//		void slotEditAll();
		void slotAppendTag(QString tag);
//		void slotFilterTag(QString tag);
		
		void slotDesactivateAll();
		void slotActivateAll();
		void slotActivate(bool act, QTreeWidgetItem * item, int column);
		void slotReloadFontList();

		void slotItemOpened(QTreeWidgetItem * item);
		void slotViewAll();
		void slotViewActivated();
		void slotFTRasterChanged();
		

		void slotUpdateTree();
		void slotRemoveCurrentItem();
		
		void slotPanoseFilter(const QMap<int,QList<int> >& filter);
		
		//lists
		void slotSelectFromFolders(const QString&);

		
	private slots:
		void slotPreviewUpdateSize(int);

		void slotRemoteFinished();


		
		
//		void slotWebLink(const QUrl & url );
//		void slotWebStart();
//		void slotWebLoad(int i);
//		void slotWebFinished(bool);

		
		void toggleFacesCheckBoxes(bool);

		void refreshActStatus(const QStringList& flist);

		
	signals:
		void faceChanged();
		void newTag(QString);
		void tagAdded(QString);
		void listChanged();

	public:
		QString defaultOrd() {return ord[0];}
		QList<FontItem*> curFonts();
		void setCurFonts(QList<FontItem*> flist);
		FontItem* selectedFont(){return theVeryFont;}

		
		QString sampleName();
//		void displayWelcomeMessage();
		
		QWebView *info();
		
		void addFilterToCrumb(QString filter);
		void setCrumb(QString text = QString());
		
		void saveSplitterState();
		void restoreSplitterState();

		void forceReloadSelection();


	protected:
		void keyPressEvent ( QKeyEvent * event ) ;
};

#endif
