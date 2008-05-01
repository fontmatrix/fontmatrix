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


#ifndef TYPOTEK_H
#define TYPOTEK_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QLabel>
#include <QThread>
#include <QTime>

#include "prefspaneldialog.h"

class QAction;
class QMenu;
class QTextEdit;
class MainViewWidget;
class FontItem;
// class TypotekAdaptator;
class QDockWidget;
class Systray;
class RemoteDir;



class typotek:public QMainWindow
{
		Q_OBJECT

	public:
		typotek();
		void initMatrix();
		~typotek();

	protected:
		void closeEvent ( QCloseEvent *event );
		void keyPressEvent ( QKeyEvent * event ) ;

	private slots:
		
		void open();
		void open(QStringList files);
		
		void printInfo();
		void printSample();
		void printChart();
		void printPlayground();
		void printFamily();
		
		void fontBook();
		void popupTagsetEditor();
		void slotActivateCurrents();
		void slotDeactivateCurrents();
		void slotEditFont();
		void about();
		void help();
// 		void slotWord();
		void slotExportFontSet();
		void slotRemoteIsReady();
		void slotRepair();
		void slotTagAll();
		void slotMainDockAreaChanged(Qt::DockWidgetArea area);
		
		
		
	public slots:
		bool save();
		void slotCloseToSystray(bool isEnabled);
		void slotUseInitialTags(bool isEnabled);
		void slotPrefsPanelDefault();
		void slotPrefsPanel(PrefsPanelDialog::PAGE page);
		void relayStartingStepIn(QString s);
		
	signals:
		void tagAdded(QString);
		void relayStartingStepOut(QString, int, QColor);
		void previewDirectionHasChanged();
// 		void wordHasChanged(QString);

	private:
		void createActions();
		void createMenus();
		void createToolBars();
		void createStatusBar();
		void readSettings();
		void writeSettings();
		bool maybeSave();
		void initDir();
		void doConnect();
		void setupDrop();

		void checkOwnDir();
		void fillTagsList();

		QTextEdit *textEdit;
		QString curFile;
		
		QDockWidget *mainDock;
		QMenu *fileMenu;
		QMenu *editMenu;
		QMenu *helpMenu;
		QToolBar *fileToolBar;
		QToolBar *editToolBar;
		QAction *newAct;
		QAction *openAct;
		QAction *saveAct;
		QAction *saveAsAct;
		QAction *exitAct;
		QAction *cutAct;
		QAction *copyAct;
		QAction *pasteAct;
		QAction *aboutAct;
		QAction *aboutQtAct;
		QAction *fontBookAct;
		QAction *tagsetAct;
		QAction *activCurAct;
		QAction *deactivCurAct;
		QAction *helpAct;
		QAction *fonteditorAct;
		QAction *prefsAct;
		QAction *exportFontSetAct;
		QAction *repairAct;
		QAction *tagAll;
		
		QMenu *printMenu;
		QAction *printInfoAct;
		QAction *printSampleAct;
		QAction *printChartAct;
		QAction *printPlaygroundAct;
		QAction *printFamilyAct;

		MainViewWidget *theMainView;

		QFile fontsdata;
		QDir ownDir;
		QDir managedDir;

		QMap<QString, QStringList> tagsMap;
		QMap<QString, QStringList> tagSetMap;
		
		QList<FontItem*> fontMap;
		QMap<QString, FontItem*> realFontMap;
		
// 		TypotekAdaptator *actAdaptator;
		
		QMap<QString,QString> m_namedSamples;
		QString m_theWord;
		
		QLabel *curFontPresentation;

		Systray *systray;

		bool useInitialTags;
		static QString fonteditorPath;
		QString templatesDir;
		double previewSize;
		bool previewRTL;
		
		void addFcDirItem(const QString &dirPath);
		QStringList getSystemFontDirs();
		
		RemoteDir *remoteDir;
		QString m_remoteTmpDir;
		
		QString m_defaultSampleName;
		
		QString mainDockArea;
	public:
		int getFontCount(){return fontMap.count(); }
		FontItem* getFont ( int i ) ;
		FontItem* getFont ( QString s );
		QList<FontItem*> getAllFonts() {return fontMap;};
		QList<FontItem*> getFonts ( QString pattern, QString field );
		QList<FontItem*> getCurrentFonts();
				
		void addTagMapEntry(QString key, QStringList value){tagsMap[key] = value;};
		void addTagSetMapEntry(QString key, QStringList value){tagSetMap[key] = value;};
		void removeTagFromSet(QString set, QString tag){tagSetMap[set].removeAll(tag);};
		void addTagToSet(QString set, QString tag){tagSetMap[set].append(tag);};
		void removeTagset(QString key){tagSetMap.remove(key);};
		QStringList tagsets(){return tagSetMap.keys();};
		QStringList tagsOfSet(QString set){return tagSetMap[set];};
		
		static QStringList tagsList;
// 		TypotekAdaptator *adaptator(){return actAdaptator;};
		
		static typotek* instance;
		static typotek* getInstance(){return instance;};
		
		QString getManagedDir(){return managedDir.absolutePath();};
		
		QFile* getFontsData(){ return &fontsdata; }
		
		void setSampleText(QString s);
		
		void presentFontName(QString s){curFontPresentation->setText(s);};
		
		void forwardUpdateView();
		
		Systray *getSystray() const {return systray;}
		void setSystrayVisible(bool);
		void showActivateAllSystray(bool);
		void systrayAllConfirmation(bool);
		void systrayTagsConfirmation(bool);

		QString namedSample(QString name);
		QStringList namedSamplesNames(){return m_namedSamples.uniqueKeys();};
		void addNamedSample(QString name, QString sample);
		void addNamedSampleFragment(QString name, QString sampleFragment);

		void changeSample(QString name, QString text);

		void setFontEditorPath(const QString &path);
		QString fontEditorPath() {return fonteditorPath;};

		bool initialTags() { return useInitialTags;};
		
		void setTemplatesDir(const QString &dir);
		QString getTemplatesDir() {return templatesDir;};
		
		void setWord(QString s, bool updateView);
		QString word(){return m_theWord;};
		void setPreviewSize(double d){ previewSize = d; }
		double getPreviewSize(){ return previewSize; }
		void setPreviewRTL(bool d);
		bool getPreviewRTL(){ return previewRTL; }
		
		void removeFontItem(QString key);
		void removeFontItem(QStringList keyList);
		
		void changeFontSizeSettings(double fSize, double lSize);
		
		void showStatusMessage(const QString &message);
		
		QString remoteTmpDir() const {return m_remoteTmpDir;}
		void setRemoteTmpDir(const QString &s);

		QString defaultSampleName(){return m_defaultSampleName;}
		
		bool insertTemporaryFont(const QString& path);
				
	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent ( QDropEvent * event );
		void showEvent ( QShowEvent * event );
		
		

	friend class Systray; // a bit ugly but i'll need access to privates
};

class LazyInit : public QThread
{
	Q_OBJECT
	public:
		void run();
	signals:
		void endOfRun();
};


#endif
