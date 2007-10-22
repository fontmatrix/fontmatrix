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

class QAction;
class QMenu;
class QTextEdit;
class MainViewWidget;
class FontItem;
class TypotekAdaptator;

class typotek:public QMainWindow
{
		Q_OBJECT

	public:
		typotek();
		~typotek();

	protected:
		void closeEvent ( QCloseEvent *event );

	private slots:
		void newFile();
		void open();
		void print();
		void fontBook();

		bool saveAs();
		void about();
		void documentWasModified();
	public slots:
		bool save();

	private:
		void createActions();
		void createMenus();
		void createToolBars();
		void createStatusBar();
		void readSettings();
		void writeSettings();
		bool maybeSave();
		void loadFile ( const QString &fileName );
		bool saveFile ( const QString &fileName );
		void setCurrentFile ( const QString &fileName );
		QString strippedName ( const QString &fullFileName );
		void initDir();

		void checkOwnDir();
		void fillTagsList();

		QTextEdit *textEdit;
		QString curFile;

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

		MainViewWidget *theMainView;

		QFile fontsdata;
		QDir ownDir;

		QMap<QString, QStringList> tagsMap;
		QList<FontItem*> fontMap;
		QMap<QString, FontItem*> realFontMap;
		
		TypotekAdaptator *actAdaptator;
	public:
		FontItem* getFont ( int i ) {return fontMap.at ( i );};
		FontItem* getFont ( QString s ) {return realFontMap.value ( s );};
		QList<FontItem*> getAllFonts() {return fontMap;};
		QList<FontItem*> getFonts ( QString pattern, QString field );
		
		
		static QStringList tagsList;
		TypotekAdaptator *adaptator(){return actAdaptator;};
		
		static typotek* instance;
		static typotek* getInstance(){return instance;};
};

#endif
