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
class ListDockWidget;
struct OTFSet;

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
		QGraphicsScene *abcScene;
		QGraphicsScene *loremScene;
		QGraphicsScene *ftScene;
		QGraphicsScene *playScene;
		QStringList ord;
		QStringList fields;
		typotek *typo;
		ListDockWidget *m_lists;
		QString faceIndex;
		QString lastIndex;
		QList<FontItem*> currentFonts;
		QString sampleText;
		QGridLayout *tagLayout;
		QString currentOrdering;
		FontItem *theVeryFont; 
		bool fontsetHasChanged;
		QGraphicsRectItem *curGlyph;
		
		void allActivation(bool act);
		void activation(FontItem* fit, bool act, bool updateTree = true);
		void fillTree();
		
		QStringList openKeys;
		QString curItemName;
		
		double sampleFontSize;
		double sampleInterSize;
		double sampleRatio;
		
		QMap<QString, QPair<int,int> > uniPlanes;
		void fillUniPlanes();
		void fillUniPlanesCombo(FontItem* item);
		
		void fillOTTree();
		OTFSet deFillOTTree();
		
		bool renderingLock;
		int fancyGlyphInUse;
		
		bool uRangeIsNotEmpty;
		
		
		void prepare(QList<FontItem*> fonts);
		QList<FontItem*> theTaggedFonts;		
		bool contextMenuReq;
		QPoint contextMenuPos;
		
		QIcon iconPS1;
		QIcon iconTTF;
		QIcon iconOTF;
		
		QString currentDownload;
		
		
	public slots:
		void slotOrderingChanged ( QString s );
		void slotFontSelected ( QTreeWidgetItem * item, int column );
		void slotFontSelectedByName(QString fname);
		void slotInfoFont();
		void slotView(bool needDeRendering = false);
		void slotShowOneGlyph();
		void slotShowAllGlyph();
		void slotSearch();
		void slotLiveSearch(const QString & text);
		void slotFontAction(QTreeWidgetItem * item, int column );
		void slotFontActionByName(const QString &fname);
		void slotFontActionByNames(QStringList fnames);
		void slotEditAll();
		void slotZoom(int z);
		void slotAppendTag(QString tag);
		void slotFilterTag(QString tag);
		void slotFilterTagset(QString set);
		void slotDesactivateAll();
		void slotActivateAll();
		void slotSetSampleText(QString);
		void slotActivate(bool act, QTreeWidgetItem * item, int column);
		void slotReloadFontList();
		void slotReloadTagsetList();
// 		void slotShowCodePoint();
		void slotSwitchAntiAlias(bool aa);
		void slotFitChanged(int i);
		void slotRefitSample();
		void slotItemOpened(QTreeWidgetItem * item);
		void slotViewAll();
		void slotViewActivated();
		void slotPlaneSelected(int);
		void slotAdjustGlyphView(int width);
		void slotFeatureChanged();
		void slotSampleChanged();
		void slotFTRasterChanged();
		void slotWantShape();
		void slotChangeScript();
// 		void slotSwitchRTL();
// 		void slotSwitchVertUD();
		void slotProgressionChanged();
		void slotUpdateGView();
		void slotUpdateSView();
		void slotUpdateRView();
		void slotEditSampleText();
		void slotRemoveCurrentItem();
		//tags
		void slotSwitchCheckState( QListWidgetItem * item );
		void slotNewTag();
		void slotContextMenu(QPoint  pos);
		void slotFinalize();
		//playground
		void slotPushOnPlayground();
		
	private slots:
		void slotLiveFontSize(double);
		void slotRemoteFinished();
		
	signals:
		void faceChanged();
		void newTag(QString);
		void activationEvent(QString);
		void tagAdded(QString);

	public:
		QString defaultOrd() {return ord[0];};
		QGraphicsScene* glyphsScene()const{return abcScene;};
		QGraphicsScene* textScene()const{return loremScene;};
		QList<FontItem*> curFonts(){return currentFonts;};
		FontItem* selectedFont(){return theVeryFont;};
		
		void reSize(double fSize, double lSize){sampleFontSize = fSize; sampleInterSize = lSize;}
		void refillSampleList();
		
	protected:
		void keyPressEvent ( QKeyEvent * event ) ;

};

#endif
