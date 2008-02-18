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


/**
	@author Pierre Marchand <pierremarc@oep-h.com>
*/
class PrefsPanelDialog : public QDialog, private Ui::PrefsPanel
{
	Q_OBJECT
	public:
		PrefsPanelDialog ( QWidget *parent );

		~PrefsPanelDialog();
		
		enum PAGE{PAGE_GENERAL,PAGE_SAMPLETEXT,PAGE_FILES};

		void initSystrayPrefs(bool hasSystray, bool isVisible, bool hasActivateAll, bool allConfirmation, bool tagConfirmation);
		void initSampleTextPrefs();
		void initFilesAndFolders();
		void showPage(PAGE page);

	private:
		void doConnect();
	private slots:
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
		void setupFontEditor(QString);
		void slotFontEditorBrowse();
		
		void setupTemplates(const QString&);
		void slotTemplatesBrowse();
		
		void slotAddRemote();
		void slotRemoveRemote();
		
		void slotSetLocalStorage(QString s);
		void slotBrowseLocalStorage();

};

#endif
