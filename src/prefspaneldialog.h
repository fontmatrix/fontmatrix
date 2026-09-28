/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PREFSPANELDIALOG_H
#define PREFSPANELDIALOG_H

#include <KPageDialog>
#include <ui_prefs_panel.h>

class QStandardItemModel;
class KPageWidgetItem;
class KMessageWidget;

/**
    @author Pierre Marchand <pierremarc@oep-h.com>
*/
class PrefsPanelDialog : public KPageDialog, private Ui::PrefsPanel
{
    Q_OBJECT
public:
    explicit PrefsPanelDialog(QWidget *parent);

    ~PrefsPanelDialog() override;

    enum PAGE {
        PAGE_GENERAL = 0,
        PAGE_SYSTRAY,
        PAGE_DISPLAY,
        PAGE_SERVICES,
        PAGE_SAMPLETEXT,
        PAGE_FILES,
        PAGE_SHORTCUTS
    };

    void initSystrayPrefs(bool hasSystray, bool isVisible, bool hasActivateAll, bool allConfirmation, bool tagConfirmation);
    void initSampleTextPrefs();
    void initFilesAndFolders();
    void initShortcuts();
    void showPage(PAGE page);

    bool event(QEvent *ev) override;
    void keyPressEvent(QKeyEvent *k) override;
    void keyReleaseEvent(QKeyEvent *k) override;
    void done(int r) override;
    static QString getKeyText(int KeyC);

private:
    void doConnect();
    QStandardItemModel *shortcutModel = nullptr;
    QDialog *m_uiHolder = nullptr;
    KMessageWidget *m_systrayUnavailable = nullptr;
    KMessageWidget *m_sampleNameWarning = nullptr;
    KPageWidgetItem *m_pageGeneral = nullptr;
    KPageWidgetItem *m_pageSystray = nullptr;
    KPageWidgetItem *m_pageDisplay = nullptr;
    KPageWidgetItem *m_pageTools = nullptr;
    KPageWidgetItem *m_pageSampleText = nullptr;
    KPageWidgetItem *m_pageFiles = nullptr;
    KPageWidgetItem *m_pageShortcuts = nullptr;

    /* For the keyboard shortcut */
    int keyCode = 0;
    QString Part0;
    QString Part1;
    QString Part2;
    QString Part3;
    QString Part4;
    void shortcutSet(const QString &shortcut);
    void reloadShortcuts();
    void setSelected(const QString &actionText);

private Q_SLOTS:

    void addSampleName();
    void deleteSampleName();
    void validateSampleName();
    void displayNamedText();

    void setSystrayVisible(bool);
    void setSystrayActivateAll(bool);
    void setSystrayAllConfirmation(bool);
    void setSystrayTagsConfirmation(bool);

    void updateWord(QString);
    void updateWordSize(double);
    void updateWordRTL(bool rtl);
    void updateWordSubtitled(bool subtitled);

    void updateChartFont(const QFont &font);

    void setupFontEditor(QString);
    void slotFontEditorBrowse();

    void setupTemplates(const QString &);
    void slotTemplatesBrowse();
    void slotRemoteDirAdd();
    void slotRemoteDirRemove();
    void slotSystemWide(bool checked);

    void slotShowImportedFonts(bool show);
    void slotSplashScreen(bool state);

    void slotChangeShortcut();
    void slotClearShortcut();
    void slotActionSelected(const QModelIndex &mi);

    void slotDictDialog();

private:
    void applySampleText();
    // 		void slotFamilyNotPreferred(bool state);
};

#endif
