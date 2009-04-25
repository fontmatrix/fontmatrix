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

#define VIEW_PAGE_FREETYPE 0
#define VIEW_PAGE_ABSOLUTE 1
#define VIEW_PAGE_OPENTYPE 0
#define VIEW_PAGE_SETTINGS 1

class QGraphicsScene;
class typotek;
class FontItem;
class QTextEdit;
class QGridLayout;
class QTreeWidgetItem;
class QGraphicsRectItem;
class QButtonGroup;
class ListDockWidget;
struct OTFSet;
class FMLayout;

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
		FMLayout *textLayout;
		QStringList ord;
		QStringList fields;
		typotek *typo;
		ListDockWidget *m_lists;
		QString faceIndex;
		QString lastIndex;
		QList<FontItem*> currentFonts;
		QList<FontItem*> orderedCurrentFonts;
		QString sampleText;
		QGridLayout *tagLayout;
		QString currentOrdering;
		FontItem *theVeryFont; 
		bool fontsetHasChanged;
		QGraphicsRectItem *curGlyph;
		bool activateByFamilyOnly;
		
		void doConnect();
		void disConnect();
		void allActivation(bool act);
		void activation(FontItem* fit, bool act, bool andUpdate = true);
		void activation(QList<FontItem*> fit, bool act);
		void fillTree();
		void updateTree(bool checkFontActive = false);
		
		
		void operateFilter(QList<FontItem*> allFiltered, const QString filterName);
		
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
		int fancyGlyphData;
		
		bool uRangeIsNotEmpty;
		
		
// 		void prepare(QList<FontItem*> fonts);
// 		QList<FontItem*> theTaggedFonts;		
// 		bool contextMenuReq;
// 		QPoint contextMenuPos;
		
		QIcon iconPS1;
		QIcon iconTTF;
		QIcon iconOTF;
		
		QString currentDownload;
		
		QUrl infoCSSUrl;
		
		QButtonGroup *radioRenderGroup;
		QButtonGroup *radioFTHintingGroup;
		
		unsigned int hinting();
		
		int toolPanelWidth;
		
	public slots:
		void slotOrderingChanged ( QString s );
		void slotFontSelected ( QTreeWidgetItem * item, int column );
		void slotFontSelectedByName(QString fname);
		void slotInfoFont();
		void slotView(bool needDeRendering = false);
		void slotShowOneGlyph();
		void slotShowAllGlyph();
		void slotSearch();
		
		void slotFontAction(QTreeWidgetItem * item, int column );
		void slotFontActionByName(const QString &fname);
		void slotFontActionByNames(QStringList fnames);
		void slotEditAll();
		void slotZoom(int z);
		void slotAppendTag(QString tag);
		void slotFilterTag(QString tag);
		
		void slotDesactivateAll();
		void slotActivateAll();
		void slotSetSampleText(QString);
		void slotActivate(bool act, QTreeWidgetItem * item, int column);
		void slotReloadFontList();
		
		void slotSwitchAntiAlias(bool aa);
		
		void slotItemOpened(QTreeWidgetItem * item);
		void slotViewAll();
		void slotViewActivated();
		void slotPlaneSelected(int);
		void slotSearchCharName();
		void slotAdjustGlyphView(int width);
		void slotFeatureChanged();
		void slotSampleChanged();
		void slotFTRasterChanged();
		void slotWantShape();
		void slotChangeScript();
		
		
		void slotProgressionChanged();
		void slotUpdateGView();
		void slotUpdateGViewSingle();
		void slotUpdateSView();
		void slotUpdateRView();
		void slotUpdateTree();
		void slotEditSampleText();
		void slotRemoveCurrentItem();
		
		void slotShowClassification();
		void slotUpdateClassDescription(const QString& ks);
		void slotPanoseFilter();
		
		
		//playground
		void slotPushOnPlayground();
		
		//lists
		void slotSelectFromFolders(const QString&);
		
		//glyphs view
		void slotShowULine(bool);
		
	private slots:
		void slotLiveFontSize();
		void slotRemoteFinished();

		void slotDefaultOTF();
		void slotResetOTF();
		
		void slotChangeViewPageSetting(bool);
		void slotChangeViewPage(QAbstractButton* );
		void slotHintChanged(int);
		
		
		void slotWebLink(const QUrl & url );
		void slotWebStart();
		void slotWebLoad(int i);
		void slotWebFinished(bool);
		
		void slotSaveClassSplitter();
		
		void toggleFacesCheckBoxes(bool);
		
	signals:
		void faceChanged();
		void newTag(QString);
		void tagAdded(QString);
		void stopLayout();
		void listChanged();

	public:
		QString defaultOrd() {return ord[0];};
		QGraphicsScene* glyphsScene()const{return abcScene;};
		QGraphicsScene* textScene()const{return loremScene;};
		QList<FontItem*> curFonts();
		void setCurFonts(QList<FontItem*> flist);
		FontItem* selectedFont(){return theVeryFont;};
		
		void reSize(double fSize, double lSize){sampleFontSize = fSize; sampleInterSize = lSize;}
		void refillSampleList();
		
		QString sampleName();
		void displayWelcomeMessage();
		
		QWebView *info();
		QGraphicsScene *currentSampleScene();
		FMPlayGround *getPlayground();
		
		void addFilterToCrumb(QString filter);
		void setCrumb(QString text = QString());
		
		QByteArray saveSplitterState();
		void restoreSplitterState();
		
	protected:
		void keyPressEvent ( QKeyEvent * event ) ;
};

#endif
