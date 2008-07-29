//
// C++ Interface: prefspaneldialog
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PREFSPANELDIALOG_H
#define PREFSPANELDIALOG_H

#include <qdialog.h>
#include <ui_prefs_panel.h>

class QStandardItemModel;

/**
	@author Pierre Marchand <pierremarc@oep-h.com>
*/
class PrefsPanelDialog : public QDialog, private Ui::PrefsPanel
{
	Q_OBJECT
	public:
		PrefsPanelDialog ( QWidget *parent );

		~PrefsPanelDialog();

		enum PAGE{PAGE_GENERAL,PAGE_SAMPLETEXT,PAGE_FILES,PAGE_SHORTCUTS};

		void initSystrayPrefs(bool hasSystray, bool isVisible, bool hasActivateAll, bool allConfirmation, bool tagConfirmation);
		void initSampleTextPrefs();
		void initFilesAndFolders();
		void initShortcuts();
		void showPage(PAGE page);

		bool event( QEvent* ev );
		void keyPressEvent(QKeyEvent *k);
		void keyReleaseEvent(QKeyEvent *k);
		static QString getKeyText(int KeyC);

	private:
		void doConnect();
		QStandardItemModel *shortcutModel;

		/* For the keyboard shortcut */
		int keyCode;
		QString Part0;
		QString Part1;
		QString Part2;
		QString Part3;
		QString Part4;
		void shortcutSet(const QString &shortcut);
		void reloadShortcuts();
		void setSelected(const QString &actionText);

	private slots:
		void slotSelectPage(QListWidgetItem * item);
		void applySampleText();

		void addSampleName();
		void validateSampleName();
		void displayNamedText();

		void setSystrayVisible(bool);
		void setSystrayActivateAll(bool);
		void setSystrayAllConfirmation(bool);
		void setSystrayTagsConfirmation(bool);

		void updateWord(QString);
		void updateWordSize(double);
		void updateWordRTL(int);
		void updateWordSubtitled(int);

		void updateChartFontFamily(const QFont & font);
		void updateChartFontSize(int);

		void setupFontEditor(QString);
		void slotFontEditorBrowse();

		void setupTemplates(const QString&);
		void slotTemplatesBrowse();

		void slotAddRemote();
		void slotRemoveRemote();

		void slotSetLocalStorage(QString s);
		void slotBrowseLocalStorage();

		void slotShowImportedFonts(int i);
		void slotFamilyNotPreferred(bool state);
		void slotSplashScreen(bool state);

		void slotChangeShortcut();
		void slotClearShortcut();
		void slotActionSelected(const QModelIndex &mi);

		void slotDictDialog();

		void slotClose();

};

#endif
