//
// C++ Implementation: prefspaneldialog
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "prefspaneldialog.h"
#include "typotek.h"
#include "shortcuts.h"
#include <QDebug>
#include <QToolTip>
#include <QSettings>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>

PrefsPanelDialog::PrefsPanelDialog(QWidget *parent)
 : QDialog(parent)
{
	//get this before anything
	double pSize = typotek::getInstance()->getPreviewSize();
	setupUi(this);
	fontEditorPath->setText(typotek::getInstance()->fontEditorPath());
	doConnect();
	systrayFrame->setCheckable(true);
	previewWord->setText(typotek::getInstance()->word());
	previewSizeSpin->setValue(pSize);
	previewIsRTL->setChecked(typotek::getInstance()->getPreviewRTL());
	initTagBox->setChecked(typotek::getInstance()->initialTags());
	showNamesBox->setChecked(typotek::getInstance()->showImportedFonts());
}


PrefsPanelDialog::~PrefsPanelDialog()
{
}

void PrefsPanelDialog::initSystrayPrefs(bool hasSystray, bool isVisible, bool hasActivateAll, bool allConfirmation, bool tagConfirmation)
{
	if (!hasSystray) {
		systrayFrame->setEnabled(false);
		systrayFrame->setToolTip(tr("Looks like your setup does not have a system tray available."));
	}
	systrayFrame->setToolTip("");
	systrayFrame->setChecked(isVisible);
	activateAllFrame->setChecked(hasActivateAll);
	activateAllConfirmation->setChecked(allConfirmation);
	tagsConfirmation->setChecked(tagConfirmation);
	QSettings settings ;
	closeToSystray->setChecked(settings.value("SystrayCloseToTray", true).toBool());
	previewSizeSpin->setValue(typotek::getInstance()->getPreviewSize());

}

void PrefsPanelDialog::initSampleTextPrefs()
{
	//At least fill the sampletext list :)
	sampleTextNamesList->addItems(typotek::getInstance()->namedSamplesNames());
	QSettings settings;
	fontSizeSpin->setValue( settings.value("SampleFontSize",12.0).toDouble() );
	interLineSpin->setValue( settings.value("SampleInterline",16.0).toDouble() );
}

void PrefsPanelDialog::initFilesAndFolders()
{
	QSettings settings;
	templatesFolder->setText(typotek::getInstance()->getTemplatesDir());
	QStringList remoteDirV(settings.value("RemoteDirectories").toStringList());
	remoteDirList->addItems(remoteDirV);
	localStorageLine->setText(typotek::getInstance()->remoteTmpDir());

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
	connect(namedSampleTextText,SIGNAL(textChanged()),this,SLOT(validateSampleName()));
	connect(addSampleTextNameButton,SIGNAL(released()),this,SLOT(addSampleName()));
	connect(newSampleTextNameText,SIGNAL(editingFinished()),this,SLOT(addSampleName()));
	connect(sampleTextNamesList,SIGNAL(currentTextChanged( const QString& )),this,SLOT(displayNamedText()));
	connect(applySampleTextButton,SIGNAL(released()),this,SLOT(applySampleText()));

	connect(systrayFrame, SIGNAL(clicked(bool)), this, SLOT(setSystrayVisible(bool)));
	connect(activateAllFrame, SIGNAL(clicked(bool)), this, SLOT(setSystrayActivateAll(bool)));
	connect(activateAllConfirmation, SIGNAL(clicked(bool)), this, SLOT(setSystrayAllConfirmation(bool)));
	connect(tagsConfirmation, SIGNAL(clicked(bool)), this, SLOT(setSystrayTagsConfirmation(bool)));
	connect(closeToSystray, SIGNAL(clicked(bool)), typotek::getInstance(), SLOT(slotCloseToSystray(bool)));

	connect(previewWord, SIGNAL(textChanged ( const QString  ) ), this, SLOT(updateWord(QString)));
	connect(previewSizeSpin, SIGNAL(valueChanged ( double  ) ), this, SLOT(updateWordSize(double)));
	connect(previewIsRTL, SIGNAL(stateChanged( int )), this, SLOT(updateWordRTL(int)));

	connect(fontEditorPath, SIGNAL(textChanged ( const QString  ) ), this, SLOT(setupFontEditor(QString)));
	connect(fontEditorBrowse, SIGNAL(clicked()), this, SLOT(slotFontEditorBrowse()));

	connect(initTagBox, SIGNAL(clicked(bool)), typotek::getInstance(), SLOT(slotUseInitialTags(bool)));
	connect(familyNameScheme,SIGNAL(stateChanged( int )),this,SLOT(slotFamilyNotPreferred(int)));

	connect(templatesDirBrowse,SIGNAL(clicked( )),this, SLOT(slotTemplatesBrowse()));
	connect(templatesFolder,SIGNAL(textChanged( const QString& )),this,SLOT(setupTemplates(const QString&)));

	connect(remoteDirAdd,SIGNAL(clicked()),this,SLOT(slotAddRemote()));
	connect(remoteDirRemove,SIGNAL(clicked()),this,SLOT(slotRemoveRemote()));
	connect(localStorageLine,SIGNAL(textChanged( const QString& )),this,SLOT(slotSetLocalStorage(QString)));
	connect(localStorageButton,SIGNAL(clicked( )),this,SLOT(slotBrowseLocalStorage()));

	connect(showNamesBox, SIGNAL(stateChanged(int)), this, SLOT(slotShowImportedFonts(int)));

	connect(clearButton, SIGNAL(clicked()), this, SLOT(slotClearShortcut()));
	connect(changeButton, SIGNAL(clicked()), this, SLOT(slotChangeShortcut()));
	connect(shortcutList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotActionSelected(const QModelIndex&)));
	connect(shortcutList, SIGNAL(activated(const QModelIndex&)), changeButton, SLOT(toggle()));

	connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
}

void PrefsPanelDialog::applySampleText()
{
	typotek::getInstance()->changeFontSizeSettings( fontSizeSpin->value(), interLineSpin->value() );
	typotek::getInstance()->forwardUpdateView();
}

void PrefsPanelDialog::addSampleName()
{
	QString n = newSampleTextNameText->text();
	if(n.isEmpty())
		return;
	if(typotek::getInstance()->namedSamplesNames().contains(n))
		return;

	typotek::getInstance()->addNamedSample(n, tr("A text"));
	sampleTextNamesList->addItem(n);
	newSampleTextNameText->clear();
// 	displayNamedText();

}


void PrefsPanelDialog::displayNamedText()
{
	QString name(sampleTextNamesList->currentItem()->text());
	qDebug() << "name is "<< name;
	QString text(typotek::getInstance()->namedSample(name));
	qDebug() << "text is " << text;
	namedSampleTextText->setPlainText(text);
}

void PrefsPanelDialog::validateSampleName()
{
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

	QSettings settings;
	settings.setValue("PreviewSize", d);
	typotek::getInstance()->setPreviewSize(d);
	typotek::getInstance()->setWord(previewWord->text(), true);
}


void PrefsPanelDialog::updateWordRTL(int rtl)
{
	bool booleanState = (rtl == Qt::Checked) ? true : false;
	QSettings settings;
	settings.setValue("PreviewRTL", booleanState);
	typotek::getInstance()->setPreviewRTL(booleanState);
}

void PrefsPanelDialog::setupFontEditor(QString s)
{
	typotek::getInstance()->setFontEditorPath(s);
}

void PrefsPanelDialog::slotFontEditorBrowse()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Select font editor"));
	if (!s.isEmpty()) {
		fontEditorPath->setText(s);
	}
}

void PrefsPanelDialog::showPage(PAGE page)
{
	if(page == PAGE_GENERAL)
		mainTab->setCurrentIndex ( 0 );
	else if(page == PAGE_SAMPLETEXT)
		mainTab->setCurrentIndex ( 1 );
	else if(page == PAGE_FILES)
		mainTab->setCurrentIndex ( 2 );
}

void PrefsPanelDialog::slotTemplatesBrowse()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Select Templates Folder"), QDir::homePath(), QFileDialog::ShowDirsOnly);
	if (!s.isEmpty()) {
		templatesFolder->setText(s);
	}
}

void PrefsPanelDialog::setupTemplates(const QString &tdir)
{
	if(!tdir.isEmpty())
		typotek::getInstance()->setTemplatesDir(tdir);
}

void PrefsPanelDialog::slotAddRemote()
{
	QString rem(newUrlText->text());
	remoteDirList->addItem(rem);
	QStringList remList();
	QSettings settings;
	QList<QVariant> tmpL (settings.value("RemoteDirectories").toList());
	tmpL << rem;
	settings.setValue("RemoteDirectories",tmpL);
	newUrlText->clear();
}

void PrefsPanelDialog::slotRemoveRemote()
{
	if(remoteDirList->currentItem())
	{
		QString url(remoteDirList->currentItem()->text());
		qDebug()<<"about to remove "<< url;
		for(int i(0);i < remoteDirList->count();++i)
		{
			if(remoteDirList->item(i)->text() == url)
				remoteDirList->takeItem(i);
		}
		QSettings settings;
		QStringList tmpL (settings.value("RemoteDirectories").toStringList());
		QStringList remoteDirStrings;
		foreach(QString s, tmpL)
		{
			if(s != url)
				remoteDirStrings << s;
			else
				qDebug() << "Exclude "<<url<< " from remote dirs";
		}
		qDebug()<<"RemoteDirectories : "<<remoteDirStrings.join(", ");
		settings.setValue("RemoteDirectories", remoteDirStrings);

	}
}

void PrefsPanelDialog::slotSetLocalStorage(QString s)
{
	typotek::getInstance()->setRemoteTmpDir(s);
}

void PrefsPanelDialog::slotBrowseLocalStorage()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Select Where remote font files will be stored"));
	if (!s.isEmpty()) {
		localStorageLine->setText(s);
	}
}

void PrefsPanelDialog::slotShowImportedFonts(int show)
{
	int opposite = Qt::Unchecked;
	if (show == Qt::Unchecked)
		opposite = Qt::Checked;
	typotek::getInstance()->showImportedFonts(opposite);
}

void PrefsPanelDialog::slotChangeShortcut()
{
	if (changeButton->isChecked())
	{
		keyCode = 0;
		Part0 = "";
		Part1 = "";
		Part2 = "";
		Part3 = "";
		Part4 = "";
		grabKeyboard();
	}
	else
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
	reloadShortcuts();
	setSelected(iText);
}

void PrefsPanelDialog::slotActionSelected(const QModelIndex &mi)
{
	QModelIndex index = shortcutList->currentIndex();
	if (!index.isValid())
		return;

	int row = index.row();
	QStandardItem *item = shortcutModel->item(row, 1);
	QString iShortcut = item->text();
	shortcutLabel->setText(iShortcut);
}


bool PrefsPanelDialog::event( QEvent* ev )
{
	bool ret = QWidget::event( ev );
	if ( ev->type() == QEvent::KeyPress )
		keyPressEvent((QKeyEvent*)ev);
	if ( ev->type() == QEvent::KeyRelease )
		keyReleaseEvent((QKeyEvent*)ev);
	return ret;
}

void PrefsPanelDialog::keyPressEvent(QKeyEvent *k)
{
	if (changeButton->isChecked())
	{
		QStringList tl;
		if (!shortcutLabel->text().isEmpty())
		{
			tl = shortcutLabel->text().split("+", QString::SkipEmptyParts);
			Part4 = tl[tl.count()-1];
			if (Part4 == tr("Alt") || Part4 == tr("Ctrl") || Part4 == tr("Shift") || Part4 == tr("Meta"))
				Part4 = "";
		}
		else
			Part4 = "";
		switch (k->key())
		{
			case Qt::Key_Meta:
				Part0 = tr("Meta+");
				keyCode |= Qt::META;
				break;
			case Qt::Key_Shift:
				Part3 = tr("Shift+");
				keyCode |= Qt::SHIFT;
				break;
			case Qt::Key_Alt:
				Part2 = tr("Alt+");
				keyCode |= Qt::ALT;
				break;
			case Qt::Key_Control:
				Part1 = tr("Ctrl+");
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
	if (changeButton->isChecked())
	{
		shortcutLabel->setText(Part0+Part1+Part2+Part3+Part4);
	}
}

void PrefsPanelDialog::keyReleaseEvent(QKeyEvent *k)
{
	if (changeButton->isChecked())
	{
		if (!shortcutLabel->text().isEmpty())
		{
			QStringList tl;
			tl = shortcutLabel->text().split("+", QString::SkipEmptyParts);
			Part4 = tl[tl.count()-1];
			if (Part4 == tr("Alt") || Part4 == tr("Ctrl") || Part4 == tr("Shift") || Part4 == tr("Meta"))
				Part4 = "";
		}
		else
			Part4 = "";
		if (k->key() == Qt::Key_Meta)
		{
			Part0 = "";
			keyCode &= ~Qt::META;
		}
		if (k->key() == Qt::Key_Shift)
		{
			Part3 = "";
			keyCode &= ~Qt::SHIFT;
		}
		if (k->key() == Qt::Key_Alt)
		{
			Part2 = "";
			keyCode &= ~Qt::ALT;
		}
		if (k->key() == Qt::Key_Control)
		{
			Part1 = "";
			keyCode &= ~Qt::CTRL;
		}
		shortcutLabel->setText(Part0+Part1+Part2+Part3+Part4);
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
	return res + QString(QKeySequence(KeyC & ~(Qt::META | Qt::CTRL | Qt::ALT | Qt::SHIFT)));
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
	if (!reserved.isEmpty()) { // shortcut is already in use
		if (QMessageBox::question(this, tr("Replace"),
             "<qt>" + tr("Shortcut is already in use for", "action name will be appended to this") +
             QString("<br/><b>%1</b>.<br/>").arg(reserved) +
             tr("Do you still want to assign it?") + "</qt>",
             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		{
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
	QList<QAction*> alist = Shortcuts::getInstance()->getActions();
	foreach(QAction *act, alist) {
		QStandardItem *iText = new QStandardItem(act->text());
		QStandardItem *iShortcut = new QStandardItem(act->shortcut().toString());
		iShortcut->setFont(font);
		QString tooltip = act->toolTip();
		QString statusTip = act->statusTip();
		QString tip = tooltip;
		if (statusTip.length() > tooltip.length())
			tip = statusTip;
		QStandardItem *iTooltip = new QStandardItem(tip);
		QList<QStandardItem *> iRow;
		iRow << iText << iShortcut << iTooltip;
		shortcutModel->appendRow(iRow);
	}
	shortcutModel->setHeaderData(0, Qt::Horizontal, tr("Action"));
	shortcutModel->setHeaderData(1, Qt::Horizontal, tr("Shortcut"));
	shortcutModel->setHeaderData(2, Qt::Horizontal, tr("Tip"));
	shortcutList->resizeColumnsToContents();
	shortcutList->resizeRowsToContents();
	shortcutList->setSortingEnabled(true);
}

void PrefsPanelDialog::setSelected(const QString &actionText)
{
	QList<QStandardItem*> ilist = shortcutModel->findItems(actionText);
	if (ilist.count() > 0) {
		int row = ilist.at(0)->row();
		shortcutList->selectRow(row);
	}
}


void PrefsPanelDialog::slotFamilyNotPreferred(int state)
{
	QSettings settings;
	if(state == Qt::Unchecked)
		settings.setValue("FamilyPreferred", false);
	else
		settings.setValue("FamilyPreferred", true);
}

