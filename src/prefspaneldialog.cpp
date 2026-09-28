/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "prefspaneldialog.h"
#include "fmhyphenator.h"
#include "fmpaths.h"
#include "fontmatrix_debug.h"
#include "shortcuts.h"
#include "typotek.h"

#include "fmactivate.h"
#include "fmconfig.h"
#include "fmportal.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageWidget>
#include <KPageWidgetItem>
#include <KSandbox>
#include <QAction>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QToolTip>
#include <QUrl>

PrefsPanelDialog::PrefsPanelDialog(QWidget *parent)
    : KPageDialog(parent)
{
    // get this before anything
    double pSize = typotek::getInstance()->getPreviewSize();

    setWindowTitle(i18nc("@title:window", "Preferences"));
    setFaceType(KPageDialog::List);
    setStandardButtons(QDialogButtonBox::Close);
    setModal(true);

    // Build the legacy QDialog UI on a hidden holder, then move each page widget
    // into the KPageDialog. Form widgets remain accessible via Ui::PrefsPanel.
    m_uiHolder = new QDialog(this);
    m_uiHolder->setVisible(false);
    setupUi(m_uiHolder);

    m_pageGeneral = addPage(page, i18nc("@title:tab", "General"));
    m_pageGeneral->setIcon(QIcon::fromTheme(QStringLiteral("preferences-other")));
    m_pageSystray = addPage(pageSystray, i18nc("@title:tab", "System tray"));
    m_pageSystray->setIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    m_pageDisplay = addPage(pageDisplay, i18nc("@title:tab", "Display"));
    m_pageDisplay->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-display")));
    m_pageTools = addPage(page_5, i18nc("@title:tab", "Tools"));
    m_pageTools->setIcon(QIcon::fromTheme(QStringLiteral("applications-utilities")));
    m_pageSampleText = addPage(page_2, i18nc("@title:tab", "Samples collection"));
    m_pageSampleText->setIcon(QIcon::fromTheme(QStringLiteral("format-text-bold")));
    m_pageFiles = addPage(page_3, i18nc("@title:tab", "Files & Folders"));
    m_pageFiles->setIcon(QIcon::fromTheme(QStringLiteral("folder")));
    m_pageShortcuts = addPage(page_4, i18nc("@title:tab", "Shortcuts"));
    m_pageShortcuts->setIcon(QIcon::fromTheme(QStringLiteral("configure-shortcuts")));

    // Inline banner shown on the System tray page when the host has no tray
    // available. Replaces a tooltip on a disabled groupbox (which most styles
    // don't render). Pushed to the top of pageSystray's grid; the existing
    // systrayFrame is bumped down a row.
    m_systrayUnavailable = new KMessageWidget(pageSystray);
    m_systrayUnavailable->setMessageType(KMessageWidget::Warning);
    m_systrayUnavailable->setIcon(QIcon::fromTheme(QStringLiteral("dialog-warning")));
    m_systrayUnavailable->setText(i18nc("@info", "This desktop does not provide a system tray. The options below have no effect."));
    m_systrayUnavailable->setCloseButtonVisible(false);
    m_systrayUnavailable->setWordWrap(true);
    m_systrayUnavailable->hide();
    if (auto *grid = qobject_cast<QGridLayout *>(pageSystray->layout())) {
        grid->removeWidget(systrayFrame);
        grid->addWidget(m_systrayUnavailable, 0, 0);
        grid->addWidget(systrayFrame, 1, 0);
    }

    // Inline validation banner on the Samples Collection page. Surfaces the
    // previously-silent failures of addSampleName() (empty input, duplicate name).
    m_sampleNameWarning = new KMessageWidget(widget);
    m_sampleNameWarning->setMessageType(KMessageWidget::Warning);
    m_sampleNameWarning->setIcon(QIcon::fromTheme(QStringLiteral("dialog-warning")));
    m_sampleNameWarning->setCloseButtonVisible(true);
    m_sampleNameWarning->setWordWrap(true);
    m_sampleNameWarning->hide();
    if (auto *grid = qobject_cast<QGridLayout *>(widget->layout()))
        grid->addWidget(m_sampleNameWarning, 3, 0, 1, 3);

    fontEditorPath->setText(typotek::getInstance()->fontEditorPath());
    // in a sandbox a path to a program of the system means nothing: the desktop chooses
    const bool sandboxed(KSandbox::isInside() && FMPortal::isAvailable());
    fontEditorNote->setVisible(sandboxed);
    fontEditorPath->setEnabled(!sandboxed);
    fontEditorBrowse->setEnabled(!sandboxed);

    systrayFrame->setCheckable(true);
    previewWord->setText(typotek::getInstance()->word());
    previewWord->setToolTip(
        i18nc("@info:tooltip",
              "You can use the following keywords to be replaced by data from fonts: <strong>&#60;name&#62;</strong> ; <strong>&#60;family&#62;</strong> ; "
              "<strong>&#60;variant&#62;</strong>"));
    previewSizeSpin->setValue(pSize);
    previewIsRTL->setChecked(typotek::getInstance()->getPreviewRTL());
    previewSubtitled->setChecked(typotek::getInstance()->getPreviewSubtitled());
    initTagBox->setChecked(typotek::getInstance()->initialTags());
    showNamesBox->setChecked(typotek::getInstance()->showImportedFonts());
    // 	familyNameScheme->setChecked ( !typotek::getInstance()->familySchemeFreetype() );

    {
        QFont chartFont(typotek::getInstance()->getChartInfoFontName());
        chartFont.setPointSize(typotek::getInstance()->getChartInfoFontSize());
        chartFontRequester->setFont(chartFont);
        chartFontRequester->setSampleText(i18nc("@info:placeholder sample text in font requester", "Aa Bb 123"));
    }

    // 	qDebug()<< "ss" << FMConfig::value("SplashScreen",false).toBool();
    splashCheck->setChecked(FMConfig::value(QStringLiteral("SplashScreen"), true).toBool());

    namedSampleTextText->setText(i18nc("@info", "Please select an item in the list or create a new one."));
    namedSampleTextText->setEnabled(false);

    doConnect();
}

PrefsPanelDialog::~PrefsPanelDialog() = default;

void PrefsPanelDialog::initSystrayPrefs(bool hasSystray, bool isVisible, bool hasActivateAll, bool allConfirmation, bool tagConfirmation)
{
    if (!hasSystray) {
        systrayFrame->setEnabled(false);
        m_systrayUnavailable->show();
    } else {
        m_systrayUnavailable->hide();
    }
    systrayFrame->setChecked(isVisible);
    activateAllFrame->setChecked(hasActivateAll);
    activateAllConfirmation->setChecked(allConfirmation);
    tagsConfirmation->setChecked(tagConfirmation);
    closeToSystray->setChecked(FMConfig::value(QStringLiteral("Systray/CloseToTray"), true).toBool());
    startToSystemTray->setChecked(FMConfig::value(QStringLiteral("Systray/StartToTray"), false).toBool());
    previewSizeSpin->setValue(typotek::getInstance()->getPreviewSize());
}

void PrefsPanelDialog::initSampleTextPrefs()
{
    // At least fill the sampletext list :)
    sampleTextNamesList->addItems(typotek::getInstance()->namedSamplesNames().value(QString("User")));
    fontSizeSpin->setValue(FMConfig::value(QStringLiteral("Sample/FontSize"), 14.0).toDouble());
    interLineSpin->setValue(FMConfig::value(QStringLiteral("Sample/Interline"), 18.0).toDouble());
    dictEdit->setText(FMConfig::value(QStringLiteral("Sample/HyphenationDict"), QLatin1String("")).toString());
    leftBox->setValue(FMConfig::value(QStringLiteral("Sample/HyphLeft"), 2).toInt());
    rightBox->setValue(FMConfig::value(QStringLiteral("Sample/HyphRight"), 3).toInt());
}

void PrefsPanelDialog::initFilesAndFolders()
{
    templatesFolder->setText(typotek::getInstance()->getTemplatesDir());
    remoteDirsList->addItems(FMConfig::value(QStringLiteral("RemoteDirectories"), QStringList()).toStringList());
#if !defined(_WIN32) && !defined(PLATFORM_APPLE)
    // only where the root helper is installed and polkit knows its action
    activationScopeBox->setVisible(FMActivate::systemScopeAvailable());
    systemWideCheck->setChecked(FMActivate::systemScope());
#else
    activationScopeBox->setVisible(false);
#endif
}

void PrefsPanelDialog::slotSystemWide([[maybe_unused]] bool checked)
{
#if !defined(_WIN32) && !defined(PLATFORM_APPLE)
    // refused, or the password dialog cancelled: the box says what is
    if (!FMActivate::getInstance()->setSystemScope(checked))
        systemWideCheck->setChecked(!checked);
#endif
}

void PrefsPanelDialog::initShortcuts()
{
    Part0 = "";
    Part1 = "";
    Part2 = "";
    Part3 = "";
    keyCode = 0;

    shortcutLabel->setText("");

    shortcutModel = new QStandardItemModel(0, 3, this);
    reloadShortcuts();
    shortcutList->setModel(shortcutModel);
    shortcutList->setShowGrid(false);
    shortcutList->setSelectionBehavior(QAbstractItemView::SelectRows);
    shortcutList->setSelectionMode(QAbstractItemView::SingleSelection);
    shortcutList->resizeColumnsToContents();
    shortcutList->resizeRowsToContents();
    shortcutList->setSortingEnabled(true);
    shortcutList->sortByColumn(0, Qt::AscendingOrder);
}

void PrefsPanelDialog::doConnect()
{
    connect(commitSample, &QPushButton::clicked, this, &PrefsPanelDialog::validateSampleName);
    connect(addSampleTextNameButton, &QPushButton::released, this, &PrefsPanelDialog::addSampleName);
    connect(newSampleTextNameText, &QLineEdit::editingFinished, this, &PrefsPanelDialog::addSampleName);
    connect(deleteSampleTextNameButton, &QPushButton::released, this, &PrefsPanelDialog::deleteSampleName);
    connect(sampleTextNamesList, &QListWidget::currentTextChanged, this, &PrefsPanelDialog::displayNamedText);
    connect(dictButton, &QToolButton::clicked, this, &PrefsPanelDialog::slotDictDialog);
    // 	connect ( applySampleTextButton,SIGNAL ( released() ),this,SLOT ( applySampleText() ) );

    connect(systrayFrame, &QGroupBox::clicked, this, &PrefsPanelDialog::setSystrayVisible);
    connect(activateAllFrame, &QGroupBox::clicked, this, &PrefsPanelDialog::setSystrayActivateAll);
    connect(activateAllConfirmation, &QCheckBox::clicked, this, &PrefsPanelDialog::setSystrayAllConfirmation);
    connect(tagsConfirmation, &QCheckBox::clicked, this, &PrefsPanelDialog::setSystrayTagsConfirmation);
    connect(closeToSystray, &QCheckBox::clicked, typotek::getInstance(), &typotek::slotCloseToSystray);
    connect(startToSystemTray, &QCheckBox::clicked, typotek::getInstance(), &typotek::slotSystrayStart);

    connect(previewWord, &QLineEdit::textChanged, this, &PrefsPanelDialog::updateWord);
    connect(previewSizeSpin, &QDoubleSpinBox::valueChanged, this, &PrefsPanelDialog::updateWordSize);
    connect(previewIsRTL, &QCheckBox::toggled, this, &PrefsPanelDialog::updateWordRTL);
    connect(previewSubtitled, &QCheckBox::toggled, this, &PrefsPanelDialog::updateWordSubtitled);

    connect(chartFontRequester, &KFontRequester::fontSelected, this, &PrefsPanelDialog::updateChartFont);

    connect(fontEditorPath, &QLineEdit::textChanged, this, &PrefsPanelDialog::setupFontEditor);
    connect(fontEditorBrowse, &QPushButton::clicked, this, &PrefsPanelDialog::slotFontEditorBrowse);

    connect(initTagBox, &QCheckBox::clicked, typotek::getInstance(), &typotek::slotUseInitialTags);
    // 	connect ( familyNameScheme,SIGNAL ( toggled ( bool ) ),this,SLOT ( slotFamilyNotPreferred ( bool ) ) );
    connect(splashCheck, &QCheckBox::toggled, this, &PrefsPanelDialog::slotSplashScreen);

    connect(templatesDirBrowse, &QPushButton::clicked, this, &PrefsPanelDialog::slotTemplatesBrowse);
    connect(templatesFolder, &QLineEdit::textChanged, this, &PrefsPanelDialog::setupTemplates);
    connect(systemWideCheck, &QCheckBox::clicked, this, &PrefsPanelDialog::slotSystemWide);
    connect(remoteDirAdd, &QPushButton::clicked, this, &PrefsPanelDialog::slotRemoteDirAdd);
    connect(remoteDirEdit, &QLineEdit::returnPressed, this, &PrefsPanelDialog::slotRemoteDirAdd);
    connect(remoteDirRemove, &QPushButton::clicked, this, &PrefsPanelDialog::slotRemoteDirRemove);

    connect(showNamesBox, &QCheckBox::toggled, this, &PrefsPanelDialog::slotShowImportedFonts);

    connect(clearButton, &QToolButton::clicked, this, &PrefsPanelDialog::slotClearShortcut);
    connect(changeButton, &QToolButton::clicked, this, &PrefsPanelDialog::slotChangeShortcut);
    connect(shortcutList, &QTableView::clicked, this, &PrefsPanelDialog::slotActionSelected);
    // connect ( shortcutList, SIGNAL ( activated ( const QModelIndex& ) ), changeButton, SLOT ( toggle() ) );
}

void PrefsPanelDialog::applySampleText()
{
    typotek::getInstance()->changeFontSizeSettings(fontSizeSpin->value(), interLineSpin->value());
    typotek::getInstance()->forwardUpdateView();
    FMHyphenator *hyphenator = typotek::getInstance()->getHyphenator();
    if (dictEdit->text().isEmpty()) {
        // no dictionary of one's own: each sample is hyphenated in its language
        FMConfig::remove(QStringLiteral("Sample/HyphenationDict"));
        FMConfig::setValue(QStringLiteral("Sample/HyphLeft"), leftBox->value());
        FMConfig::setValue(QStringLiteral("Sample/HyphRight"), rightBox->value());
        hyphenator->unload();
        typotek::getInstance()->updateHyphenation();
    } else if (hyphenator->loadDict(dictEdit->text(), leftBox->value(), rightBox->value())) {
        FMConfig::setValue(QStringLiteral("Sample/HyphenationDict"), dictEdit->text());
        FMConfig::setValue(QStringLiteral("Sample/HyphLeft"), leftBox->value());
        FMConfig::setValue(QStringLiteral("Sample/HyphRight"), rightBox->value());

    } else // use the previous values
    {
        dictEdit->setText(FMConfig::value(QStringLiteral("Sample/HyphenationDict"), QLatin1String("")).toString());
        leftBox->setValue(FMConfig::value(QStringLiteral("Sample/HyphLeft"), 2).toInt());
        rightBox->setValue(FMConfig::value(QStringLiteral("Sample/HyphRight"), 3).toInt());
        FMConfig::setValue(QStringLiteral("Sample/HyphenationDict"), QLatin1String(""));
        FMConfig::setValue(QStringLiteral("Sample/HyphLeft"), 2);
        FMConfig::setValue(QStringLiteral("Sample/HyphRight"), 3);
        // the failed load dropped the dictionary in use: back to the sample's own
        typotek::getInstance()->updateHyphenation();
    }
}

void PrefsPanelDialog::addSampleName()
{
    QString n = newSampleTextNameText->text();
    if (n.isEmpty()) {
        m_sampleNameWarning->setText(i18nc("@info:status validation", "Please enter a name for the new sample."));
        m_sampleNameWarning->animatedShow();
        return;
    }
    if (typotek::getInstance()->namedSamplesNames().contains(n)) {
        m_sampleNameWarning->setText(i18nc("@info:status validation", "A sample named \"%1\" already exists.", n));
        m_sampleNameWarning->animatedShow();
        return;
    }

    m_sampleNameWarning->animatedHide();
    typotek::getInstance()->addNamedSample(n, i18nc("A default sample text inserted when creating a new sample", "Sample Text"));
    sampleTextNamesList->addItem(n);
    newSampleTextNameText->clear();
    // 	displayNamedText();
}

void PrefsPanelDialog::deleteSampleName()
{
    QList<QListWidgetItem *> sel(sampleTextNamesList->selectedItems());
    if (sel.isEmpty())
        return;

    QString sampleKey(sel[0]->text());
    QString message(i18nc("the name of a sample text will be append to the string", "Do you confirm that you want to remove:") + " \"%1\"");

    if (KMessageBox::warningContinueCancel(this,
                                           message.arg(sampleKey),
                                           i18nc("@title:window", "Remove Sample Text"),
                                           KStandardGuiItem::remove(),
                                           KStandardGuiItem::cancel(),
                                           QString(),
                                           KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous))
        == KMessageBox::Continue) {
        QListWidgetItem *it(sampleTextNamesList->takeItem(sampleTextNamesList->row(sel[0])));
        if (it)
            delete it;
        typotek::getInstance()->removeNamedSample(sampleKey);
        qCDebug(FONTMATRIX_LOG) << "Removed" << sampleKey;
    } else
        qCDebug(FONTMATRIX_LOG) << "Did not removed" << sampleKey;
}

void PrefsPanelDialog::displayNamedText()
{
    namedSampleTextText->setEnabled(true);
    QString name(sampleTextNamesList->currentItem()->text());
    //	qDebug() << "name is "<< name;
    QString text(typotek::getInstance()->namedSample(QString("User::") + name));
    //	qDebug() << "text is " << text;
    namedSampleTextText->setPlainText(text);
}

void PrefsPanelDialog::validateSampleName()
{
    if (!sampleTextNamesList->currentItem())
        return;
    typotek::getInstance()->changeSample(sampleTextNamesList->currentItem()->text(), namedSampleTextText->toPlainText());
}

void PrefsPanelDialog::setSystrayVisible(bool isVisible)
{
    typotek::getInstance()->setSystrayVisible(isVisible);
}

void PrefsPanelDialog::setSystrayActivateAll(bool isVisible)
{
    typotek::getInstance()->showActivateAllSystray(isVisible);
}

void PrefsPanelDialog::setSystrayAllConfirmation(bool isEnabled)
{
    typotek::getInstance()->systrayAllConfirmation(isEnabled);
}

void PrefsPanelDialog::setSystrayTagsConfirmation(bool isEnabled)
{
    typotek::getInstance()->systrayTagsConfirmation(isEnabled);
}

void PrefsPanelDialog::updateWord(QString s)
{
    typotek::getInstance()->setPreviewSize(previewSizeSpin->value());
    typotek::getInstance()->setWord(s, true);
}

void PrefsPanelDialog::updateWordSize(double d)
{
    FMConfig::setValue(QStringLiteral("Preview/Size"), d);
    typotek::getInstance()->setPreviewSize(d);
    typotek::getInstance()->setWord(previewWord->text(), true);
}

void PrefsPanelDialog::updateWordRTL(bool rtl)
{
    FMConfig::setValue(QStringLiteral("Preview/RTL"), rtl);
    typotek::getInstance()->setPreviewRTL(rtl);
}

void PrefsPanelDialog::updateWordSubtitled(bool subtitled)
{
    FMConfig::setValue(QStringLiteral("Preview/Subtitled"), subtitled);
    typotek::getInstance()->setPreviewSubtitled(subtitled);
}

void PrefsPanelDialog::setupFontEditor(QString s)
{
    typotek::getInstance()->setFontEditorPath(s);
}

void PrefsPanelDialog::slotFontEditorBrowse()
{
    QString s = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Select font editor"));
    if (!s.isEmpty()) {
        fontEditorPath->setText(s);
    }
}

void PrefsPanelDialog::showPage(PAGE page)
{
    KPageWidgetItem *target = nullptr;
    switch (page) {
    case PAGE_GENERAL:
        target = m_pageGeneral;
        break;
    case PAGE_SYSTRAY:
        target = m_pageSystray;
        break;
    case PAGE_DISPLAY:
        target = m_pageDisplay;
        break;
    case PAGE_SERVICES:
        target = m_pageTools;
        break;
    case PAGE_SAMPLETEXT:
        target = m_pageSampleText;
        break;
    case PAGE_FILES:
        target = m_pageFiles;
        break;
    case PAGE_SHORTCUTS:
        target = m_pageShortcuts;
        break;
    }
    if (target)
        setCurrentPage(target);
}

void PrefsPanelDialog::slotRemoteDirAdd()
{
    const QString text(remoteDirEdit->text().trimmed());
    const QUrl url(text);
    if (text.isEmpty())
        return;
    if (!url.isValid() || (url.scheme() != QLatin1String("http") && url.scheme() != QLatin1String("https"))) {
        KMessageBox::error(this, i18nc("@info", "%1 is not the address of a web directory.", text));
        return;
    }
    QStringList dirs(FMConfig::value(QStringLiteral("RemoteDirectories"), QStringList()).toStringList());
    if (dirs.contains(text))
        return;
    dirs << text;
    FMConfig::setValue(QStringLiteral("RemoteDirectories"), dirs);
    remoteDirsList->addItem(text);
    remoteDirEdit->clear();
    typotek::getInstance()->fetchRemoteDirectories(QStringList() << text);
}

void PrefsPanelDialog::slotRemoteDirRemove()
{
    const QList<QListWidgetItem *> selected(remoteDirsList->selectedItems());
    if (selected.isEmpty())
        return;
    QStringList dirs(FMConfig::value(QStringLiteral("RemoteDirectories"), QStringList()).toStringList());
    for (QListWidgetItem *item : selected) {
        dirs.removeAll(item->text());
        delete item;
    }
    FMConfig::setValue(QStringLiteral("RemoteDirectories"), dirs);
}

void PrefsPanelDialog::slotTemplatesBrowse()
{
    QString s = QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Select Templates Folder"), QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!s.isEmpty()) {
        templatesFolder->setText(s);
    }
}

void PrefsPanelDialog::setupTemplates(const QString &tdir)
{
    if (!tdir.isEmpty())
        typotek::getInstance()->setTemplatesDir(tdir);
}

void PrefsPanelDialog::slotShowImportedFonts(bool show)
{
    typotek::getInstance()->setImportedFontsHidden(!show);
}

void PrefsPanelDialog::slotChangeShortcut()
{
    if (changeButton->isChecked()) {
        keyCode = 0;
        Part0 = "";
        Part1 = "";
        Part2 = "";
        Part3 = "";
        Part4 = "";
        grabKeyboard();
    } else
        releaseKeyboard();
}

void PrefsPanelDialog::slotClearShortcut()
{
    QModelIndex index = shortcutList->currentIndex();
    if (!index.isValid())
        return;

    int row = index.row();
    QStandardItem *item = shortcutModel->item(row, 0);
    QString iText = item->text();

    Shortcuts::getInstance()->clearShortcut(iText);
    shortcutLabel->clear();
    reloadShortcuts();
    setSelected(iText);
}

void PrefsPanelDialog::slotActionSelected(const QModelIndex &)
{
    QModelIndex index = shortcutList->currentIndex();
    if (!index.isValid())
        return;

    int row = index.row();
    QStandardItem *item = shortcutModel->item(row, 1);
    QString iShortcut = item->text();
    shortcutLabel->setText(iShortcut);
}

bool PrefsPanelDialog::event(QEvent *ev)
{
    bool ret = QWidget::event(ev);
    if (ev->type() == QEvent::KeyPress)
        keyPressEvent((QKeyEvent *)ev);
    if (ev->type() == QEvent::KeyRelease)
        keyReleaseEvent((QKeyEvent *)ev);
    return ret;
}

void PrefsPanelDialog::keyPressEvent(QKeyEvent *k)
{
    if (changeButton->isChecked()) {
        QStringList tl;
        if (!shortcutLabel->text().isEmpty()) {
            tl = shortcutLabel->text().split("+", Qt::SkipEmptyParts);
            Part4 = tl[tl.count() - 1];
            if (Part4 == i18n("Alt") || Part4 == i18n("Ctrl") || Part4 == i18n("Shift") || Part4 == i18n("Meta"))
                Part4 = "";
        } else
            Part4 = "";
        switch (k->key()) {
        case Qt::Key_Meta:
            Part0 = i18n("Meta+");
            keyCode |= Qt::META;
            break;
        case Qt::Key_Shift:
            Part3 = i18n("Shift+");
            keyCode |= Qt::SHIFT;
            break;
        case Qt::Key_Alt:
            Part2 = i18n("Alt+");
            keyCode |= Qt::ALT;
            break;
        case Qt::Key_Control:
            Part1 = i18n("Ctrl+");
            keyCode |= Qt::CTRL;
            break;
        default:
            keyCode |= k->key();
            shortcutLabel->setText(getKeyText(keyCode));
            changeButton->setChecked(false);
            releaseKeyboard();
            shortcutSet(shortcutLabel->text());
        }
    }
    if (changeButton->isChecked()) {
        shortcutLabel->setText(Part0 + Part1 + Part2 + Part3 + Part4);
    }
}

void PrefsPanelDialog::keyReleaseEvent(QKeyEvent *k)
{
    if (changeButton->isChecked()) {
        if (!shortcutLabel->text().isEmpty()) {
            QStringList tl;
            tl = shortcutLabel->text().split("+", Qt::SkipEmptyParts);
            Part4 = tl[tl.count() - 1];
            if (Part4 == i18n("Alt") || Part4 == i18n("Ctrl") || Part4 == i18n("Shift") || Part4 == i18n("Meta"))
                Part4 = "";
        } else
            Part4 = "";
        if (k->key() == Qt::Key_Meta) {
            Part0 = "";
            keyCode &= ~Qt::META;
        }
        if (k->key() == Qt::Key_Shift) {
            Part3 = "";
            keyCode &= ~Qt::SHIFT;
        }
        if (k->key() == Qt::Key_Alt) {
            Part2 = "";
            keyCode &= ~Qt::ALT;
        }
        if (k->key() == Qt::Key_Control) {
            Part1 = "";
            keyCode &= ~Qt::CTRL;
        }
        shortcutLabel->setText(Part0 + Part1 + Part2 + Part3 + Part4);
    }
}

QString PrefsPanelDialog::getKeyText(int KeyC)
{
    if ((KeyC & ~(Qt::META | Qt::CTRL | Qt::ALT | Qt::SHIFT)) == 0)
        return "";
    // on OSX Qt translates modifiers to forsaken symbols, arrows and the like
    // we prefer plain English
    QString res;
    if ((KeyC & Qt::META) != 0)
        res += "Meta+";
    if ((KeyC & Qt::CTRL) != 0)
        res += "Ctrl+";
    if ((KeyC & Qt::ALT) != 0)
        res += "Alt+";
    if ((KeyC & Qt::SHIFT) != 0)
        res += "Shift+";
    return res + QString(QKeySequence(KeyC & ~(Qt::META | Qt::CTRL | Qt::ALT | Qt::SHIFT)).toString());
}

void PrefsPanelDialog::shortcutSet(const QString &shortcut)
{
    QModelIndex index = shortcutList->currentIndex();
    if (!index.isValid())
        return;

    int row = index.row();
    QStandardItem *item = shortcutModel->item(row, 0);
    QString iText = item->text();

    Shortcuts *tmp = Shortcuts::getInstance();
    QString reserved = tmp->isReserved(shortcut, iText);
    if (!reserved.isEmpty()) // shortcut is already in use
    {
        if (KMessageBox::questionTwoActions(this,
                                            "<qt>" + i18nc("action name will be appended to this", "Shortcut is already in use for")
                                                + QString("<br/><b>%1</b>.<br/>").arg(reserved) + i18nc("@info", "Do you still want to assign it?") + "</qt>",
                                            i18nc("@title:window", "Replace"),
                                            KGuiItem(i18nc("@action:button", "Reassign")),
                                            KStandardGuiItem::cancel())
            == KMessageBox::PrimaryAction) {
            tmp->clearShortcut(reserved);
        } else {
            return; // user choose not to replace an existing shortcut
        }
    }
    tmp->setShortcut(shortcut, iText);
    reloadShortcuts();
    setSelected(iText);
}

void PrefsPanelDialog::reloadShortcuts()
{
    QFont font(shortcutList->font());
    font.setBold(true);
    shortcutModel->clear();
    QList<QAction *> alist = Shortcuts::getInstance()->getActions();
    Shortcuts *scuts = Shortcuts::getInstance();
    for (auto *act : std::as_const(alist)) {
        auto iText = new QStandardItem(scuts->cleanName(act->text()));
        auto iShortcut = new QStandardItem(act->shortcut().toString());
        iShortcut->setFont(font);
        QString tooltip = act->toolTip();
        QString statusTip = act->statusTip();
        QString tip = tooltip;
        if (statusTip.length() > tooltip.length())
            tip = statusTip;
        auto iTooltip = new QStandardItem(tip);
        QList<QStandardItem *> iRow;
        iRow << iText << iShortcut << iTooltip;
        shortcutModel->appendRow(iRow);
    }
    shortcutModel->setHeaderData(0, Qt::Horizontal, i18nc("@title:column", "Action"));
    shortcutModel->setHeaderData(1, Qt::Horizontal, i18nc("@title:column", "Shortcut"));
    shortcutModel->setHeaderData(2, Qt::Horizontal, i18nc("@title:column", "Tip"));
    shortcutList->resizeColumnsToContents();
    shortcutList->resizeRowsToContents();
    shortcutList->setSortingEnabled(true);
}

void PrefsPanelDialog::setSelected(const QString &actionText)
{
    QList<QStandardItem *> ilist = shortcutModel->findItems(Shortcuts::getInstance()->cleanName(actionText));
    if (ilist.count() > 0) {
        int row = ilist.at(0)->row();
        shortcutList->selectRow(row);
    }
}

// void PrefsPanelDialog::slotFamilyNotPreferred ( bool state )
// {
// 	qDebug() <<"slotFamilyNotPreferred("<< state <<")";
// 	QSettings settings;
// 	settings.setValue ( "FamilyPreferred", !state );
// 	typotek::getInstance()->setFamilySchemeFreetype ( !state );
// }

void PrefsPanelDialog::slotSplashScreen(bool state)
{
    // 	qDebug() <<"slotSplashScreen("<< state <<")";
    FMConfig::setValue(QStringLiteral("SplashScreen"), state);
}

void PrefsPanelDialog::slotDictDialog()
{
    QString s = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Select hyphenation dictionary"), QDir::homePath());
    if (!s.isEmpty())
        dictEdit->setText(s);
}

void PrefsPanelDialog::done(int r)
{
    applySampleText();
    KPageDialog::done(r);
}

void PrefsPanelDialog::updateChartFont(const QFont &font)
{
    // Chart subtitle consumers (fontitem.cpp) read family + size only;
    // style attributes from the requester are intentionally not persisted.
    const int size = font.pointSize() > 0 ? font.pointSize() : qRound(font.pointSizeF());

    FMConfig::setValue(QStringLiteral("ChartInfoFontFamily"), font.family());
    FMConfig::setValue(QStringLiteral("ChartInfoFontSize"), size);

    typotek::getInstance()->setChartInfoFontName(font.family());
    typotek::getInstance()->setChartInfoFontSize(size);
}

#include "moc_prefspaneldialog.cpp"
