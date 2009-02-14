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
class FMHyphenator;
class QProgressBar;
class HelpWidget;


class typotek:public QMainWindow
{
		Q_OBJECT

		static typotek* instance;
		static bool matrix;
		typotek();
		~typotek();
	public:
		static typotek* getInstance();
		void initMatrix();

	protected:
		void closeEvent ( QCloseEvent *event );
		void keyPressEvent ( QKeyEvent * event ) ;

	private slots:
		void printInfo();
		void printSample();
		void printChart();
		void printPlayground();
		void printFamily();

		void fontBook();
		void slotActivateCurrents();
		void slotDeactivateCurrents();
		void slotEditFont();
		void about();
		void helpBegin();
		void helpEnd();
		void slotExportFontSet();
		void slotRemoteIsReady();
		void slotRepair();
		void slotTagAll();
		void slotMainDockAreaChanged(Qt::DockWidgetArea area);
		void slotTagsDockAreaChanged(Qt::DockWidgetArea area);
		void slotSwitchLayOptVisible();
		void slotUpdateLayOptStatus();
		void slotShowTTTables();
		void slotEditPanose();
		void slotDumpInfo();
#ifdef HAVE_PYTHONQT
		void slotExecScript();
		void slotExecLastScript();
		void slotExecRecentScript();
#endif

	public slots:
		void open( QString path = QString(), bool announce = true, bool collect = false );
		void openList( QStringList files );
		bool save();
		void slotCloseToSystray(bool isEnabled);
		void slotSystrayStart(bool isEnabled);
		void slotUseInitialTags(bool isEnabled);
		void showImportedFonts(int show);
		bool showImportedFonts();
		void slotPrefsPanelDefault();
		void slotPrefsPanel(PrefsPanelDialog::PAGE page);
		void relayStartingStepIn(QString s);
		void showToltalFilteredFonts();

		void hide();
		void show();

	signals:
		void tagAdded(QString);
		void relayStartingStepOut(QString, int, QColor);
		void previewHasChanged();

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
		QDockWidget *tagsDock;
		QMenu *fileMenu;
		QMenu *editMenu;
		QMenu *browseMenu;
		QMenu *viewMenu;
#ifdef HAVE_PYTHONQT
		QMenu *scriptMenu;
#endif
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
		QAction *activCurAct;
		QAction *deactivCurAct;
		QAction *helpAct;
		QAction *fonteditorAct;
		QAction *prefsAct;
		QAction *exportFontSetAct;
		QAction *repairAct;
		QAction *tagAll;
		QAction *showTTTAct;
		QAction *editPanoseAct;
		QAction *dumpInfoAct;
#ifdef HAVE_PYTHONQT
		QAction *execScriptAct;
		QAction *execLastScriptAct;
		QString lastScript;
		QMap<QAction*, QString> recentScripts;
#endif

		QAction *nextFamily;
		QAction *previousFamily;
		QAction *nextFont;
		QAction *previousFont;

		QMenu *printMenu;
		QAction *printInfoAct;
		QAction *printSampleAct;
		QAction *printChartAct;
		QAction *printPlaygroundAct;
		QAction *printFamilyAct;

		HelpWidget *theHelp;

		QAction *layOptAct;

		QProgressBar *statusProgressBar;

		MainViewWidget *theMainView;

		QFile ResourceFile;
		QDir ownDir;
		QDir managedDir;

		QMap<QString,QString> m_namedSamples;
		QString m_theWord;

		QLabel *curFontPresentation;
		QLabel *countFilteredFonts;

		Systray *systray;

		bool useInitialTags;
		bool showFontListDialog;
		static QString fonteditorPath;
		QString templatesDir;
		double previewSize;
		bool previewRTL;
		bool previewSubtitled;
		bool m_familySchemeFreetype;
		QString m_welcomeURL;

		void addFcDirItem(const QString &dirPath);
		QStringList getSystemFontDirs();
		QStringList sysFontList;

		RemoteDir *remoteDir;
		QString m_remoteTmpDir;

		QString m_defaultSampleName;

		QString mainDockArea;
		QString tagsDockArea;
		QRect mainDockGeometry;
		QRect tagsDockGeometry;

		FMHyphenator *hyphenator;

		QString defaultOTFScript;
		QString defaultOTFLang;
		QStringList defaultOTFGPOS;
		QStringList defaultOTFGSUB;

		int chartInfoFontSize;
		QString chartInfoFontName;

		double previewInfoFontSize;

		QString databaseDriver;
		QString databaseHostname;
		QString databaseDbName;
		QString databaseUser;
		QString databasePassword;

		int panoseMatchTreshold;

		QString webBrowser;
		QString webBrowserOptions;

	public:
		bool isSysFont(FontItem* f);
		QList<FontItem*> getCurrentFonts();
		FontItem* getSelectedFont();
		void resetFilter();

		QString getManagedDir(){return managedDir.absolutePath();};

		QFile* getResourceFile(){ return &ResourceFile; }

		void setSampleText(QString s);

		void presentFontName(QString s);

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
		void removeNamedSample(const QString& key);

		void changeSample(QString name, QString text);

		void setFontEditorPath(const QString &path);
		QString fontEditorPath() {return fonteditorPath;};

		bool initialTags() { return useInitialTags;};

		void setTemplatesDir(const QString &dir);
		QString getTemplatesDir() {return templatesDir;};

		void setWord(QString s, bool updateView);
		QString word(){return m_theWord;};
		void setPreviewSize(double d);
		double getPreviewSize(){ return previewSize; }
		void setPreviewRTL(bool d);
		bool getPreviewRTL(){ return previewRTL; }
		void setPreviewSubtitled(bool d);
		bool getPreviewSubtitled(){ return previewSubtitled; }

		void removeFontItem(QString key);
		void removeFontItem(QStringList keyList);

		void changeFontSizeSettings(double fSize, double lSize);

		void showStatusMessage(const QString &message);

		QString remoteTmpDir() const {return m_remoteTmpDir;}
		void setRemoteTmpDir(const QString &s);

		QString defaultSampleName(){return m_defaultSampleName;}



	bool familySchemeFreetype() const{return m_familySchemeFreetype;}
	void setFamilySchemeFreetype ( bool theValue ){m_familySchemeFreetype = theValue;}

	QString welcomeURL() const{return m_welcomeURL;}

	FMHyphenator* getHyphenator() const;

	void setDefaultOTFScript ( const QString& theValue );
	QString getDefaultOTFScript() const;
	void setDefaultOTFLang ( const QString& theValue );
	QString getDefaultOTFLang() const;
	void setDefaultOTFGPOS ( const QStringList& theValue );
	QStringList getDefaultOTFGPOS() const;
	void setDefaultOTFGSUB ( const QStringList& theValue );
	QStringList getDefaultOTFGSUB() const;

	void startProgressJob(int max);
	void runProgressJob(int i = 0);
	void endProgressJob();

	int getChartInfoFontSize() const{return chartInfoFontSize;}
	QString getChartInfoFontName() const{return chartInfoFontName;}

	void setChartInfoFontSize ( int theValue ){chartInfoFontSize = theValue;}
	void setChartInfoFontName ( const QString& theValue ){chartInfoFontName = theValue;}

	MainViewWidget* getTheMainView() const{return theMainView;}

	void setDatabaseDriver ( const QString& theValue ){databaseDriver = theValue;}
	QString getDatabaseDriver() const{return databaseDriver;}

	void setDatabaseHostname ( const QString& theValue ){databaseHostname = theValue;}
	QString getDatabaseHostname() const{return databaseHostname;}

	void setDatabaseDbName ( const QString& theValue ){databaseDbName = theValue;}
	QString getDatabaseDbName() const{return databaseDbName;}

	void setDatabaseUser ( const QString& theValue ){databaseUser = theValue;}
	QString getDatabaseUser() const{return databaseUser;}

	void setDatabasePassword ( const QString& theValue ){databasePassword = theValue;}
	QString getDatabasePassword() const{return databasePassword;}

	void setPanoseMatchTreshold ( int theValue );
	int getPanoseMatchTreshold() const;

	void setWebBrowser ( const QString& theValue );
	QString getWebBrowser() const;
	void setWebBrowserOptions ( const QString& theValue );
	QString getWebBrowserOptions() const;

	double getPreviewInfoFontSize() const{return previewInfoFontSize;}





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
