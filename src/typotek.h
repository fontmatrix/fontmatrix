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

#include "prefspaneldialog.h"

class QAction;
class QMenu;
class QTextEdit;
class MainViewWidget;
class FontItem;
// class TypotekAdaptator;
class QDockWidget;
class Systray;



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
		void print();
		void fontBook();
		void popupTagsetEditor();
		void slotActivateCurrents();
		void slotDeactivateCurrents();
		void slotEditFont();
		void about();
		void help();
// 		void slotWord();
		
		
	public slots:
		bool save();
		void slotCloseToSystray(bool isEnabled);
		void slotUseInitialTags(bool isEnabled);
		void slotPrefsPanelDefault();
		void slotPrefsPanel(PrefsPanelDialog::PAGE page);
		
	signals:
		void tagAdded(QString);
		void relayStartingStep(QString, int, QColor);
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
		QAction *printAct;
		QAction *fontBookAct;
		QAction *tagsetAct;
		QAction *activCurAct;
		QAction *deactivCurAct;
		QAction *helpAct;
		QAction *fonteditorAct;
		QAction *prefsAction;

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

	public:
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
		
		void setSampleText(QString s);
				
		void setWord(QString s, bool updateView);
		QString word(){return m_theWord;};
		
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
		
		

	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent ( QDropEvent * event );

	friend class Systray; // a bit ugly but i'll need access to privates
};



#endif
