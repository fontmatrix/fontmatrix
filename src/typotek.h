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
class BrowserWidget;
class FontItem;
// class TypotekAdaptator;
class QDockWidget;
class Systray;
class RemoteDir;
class FMHyphenator;
class QProgressBar;
// class HelpWidget;
class HelpBrowser;
class DataLoader;
class FloatingWidget;
class QStackedWidget;

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
	void postInit();

protected:
	void closeEvent ( QCloseEvent *event );
	void keyPressEvent ( QKeyEvent * event ) ;

private slots:
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
	void slotDockAreaChanged(Qt::DockWidgetArea area);
//	void slotSwitchLayOptVisible();
//	void slotUpdateLayOptStatus();
	void slotShowTTTables();
	void slotEditPanose();
	void slotDumpInfo();
	void slotReloadFiltered();
	void slotReloadSingle();

	void slotExecScript();
	void slotExecLastScript();
	void slotExecRecentScript();
	void slotSwitchScriptConsole();
	void slotUpdateScriptConsoleStatus();

	void slotExtractFont();
	void slotMatchRaster();

public slots:
	void open( QString path = QString(), bool recursive = true, bool announce = true, bool collect = false );
	void importFiles();
	void openList( QStringList files );
	void slotCloseToSystray(bool isEnabled);
	void slotSystrayStart(bool isEnabled);
	void slotUseInitialTags(bool isEnabled);
	void showImportedFonts(int show);
	bool showImportedFonts();
	void slotPrefsPanelDefault();
	void slotPrefsPanel(PrefsPanelDialog::PAGE page);
	void relayStartingStepIn(QString s);
	void showToltalFilteredFonts();
	void updateFloatingStatus();
	void closeAllFloatings();
	void showAllFloatings();
	void hideAllFloatings();
	void toggleMainView(bool v);
	void pushObject(QObject* o);

	void hide();
	void show();

signals:
	void relayStartingStepOut(QString, int, QColor);
	void previewHasChanged();
	void newFontsArrived();

private:
	void installDock(const QString& id, const QString& name, QWidget *w, const QString& tip=QString() );
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

	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *servicesMenu;
	QMenu *viewMenu;
#ifdef HAVE_PYTHONQT
	QMenu *scriptMenu;
#endif
	QMenu *helpMenu;
	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QAction *newAct;
	QAction *openAct;
	QAction *importFilesAction;
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
	QAction *reloadAct;
	QAction *reloadSingleAct;
#ifdef HAVE_PYTHONQT
	QAction *execScriptAct;
	QAction *execLastScriptAct;
	QString lastScript;
	QMap<QAction*, QString> recentScripts;
	QAction *scriptConsoleAct;
#endif

	QAction *extractFontAction;
	QAction *matchRasterAct;

	QAction *playAction;
	QAction *compareAction;
	QAction *closeAllFloat;
	QAction *showAllFloat;
	QAction *hideAllFloat;
	QAction *floatSep;

	// 		HelpWidget *theHelp;
	HelpBrowser *theHelp;

//	QAction *layOptAct;

	QProgressBar *statusProgressBar;

	QStackedWidget * mainStack;
	MainViewWidget *theMainView;
	BrowserWidget * theBrowser;

	QFile ResourceFile;
	QDir ownDir;
	QDir managedDir;

	DataLoader * dataLoader;
	//		QMap<QString,QString> m_namedSamples;
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
	QString m_sysTagName;

	void addFcDirItem(const QString &dirPath);
	QStringList getSystemFontDirs();
	QStringList sysFontList;

	RemoteDir *remoteDir;
	QString m_remoteTmpDir;

	QMap<QString, QDockWidget*>  dockWidget;
	QMap<QString, QString> dockArea;
	QMap<QString, bool> dockVisible;
	QMap<QString, QRect> dockGeometry;

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

	QString infoStyle;

	double m_dpiX;
	double m_dpiY;

	QMap<FloatingWidget*, QAction*> floatingWidgets;
	QMap<FloatingWidget*, bool> visibleFloatingWidgets;
	bool playVisible;

	QString currentNamedSample;

	QToolButton * toggleMainViewButton;

public:
	bool isSysFont(FontItem* f);
	FontItem* getSelectedFont();
	void resetFilter();


	QString getManagedDir(){return managedDir.absolutePath();}

	QFile* getResourceFile(){ return &ResourceFile; }

	void setSampleText(QString s);

	void presentFontName(QString s);

	void forwardUpdateView();

	// TODO there is a lot of things here which MUST go to an independent PrefsManager class

	Systray *getSystray() const {return systray;}
	void setSystrayVisible(bool);
	void showActivateAllSystray(bool);
	void systrayAllConfirmation(bool);
	void systrayTagsConfirmation(bool);

	// Samples
	QString namedSample(QString name = QString());
	QMap<QString,QList<QString> > namedSamplesNames();
	void addNamedSample(QString name, QString sample);
	void removeNamedSample(const QString& key);
	void changeSample(QString name, QString text);
	QString defaultSampleName();

	void setFontEditorPath(const QString &path);
	QString fontEditorPath() {return fonteditorPath;}

	bool initialTags() { return useInitialTags;}

	void setTemplatesDir(const QString &dir);
	QString getTemplatesDir() {return templatesDir;}

	void setWord(QString s, bool updateView);
	QString word(FontItem * item = 0, const QString& alt = QString());
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

	QDir getOwnDir() const{return ownDir;}

	void setInfoStyle ( const QString& theValue );
	QString getInfoStyle() const{ return infoStyle; }

	QString getSysTagName() const { return m_sysTagName; }

	double getDpiX() const {return m_dpiX;}
	double getDpiY() const {return m_dpiY;}
	
	
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
