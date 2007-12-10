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
#include <QDebug>
#include <QToolTip>

PrefsPanelDialog::PrefsPanelDialog(QWidget *parent)
 : QDialog(parent)
{
	setupUi(this);
	QString st = typotek::getInstance()->namedSample("default");
// 	qDebug()<< st;
	sampleTextDefaultText->setPlainText(st);
	doConnect();
	systrayFrame->setCheckable(true);
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
}

void PrefsPanelDialog::doConnect()
{
	connect(applySampleTextButton,SIGNAL(released()),this,SLOT(applySampleText()));

	connect(systrayFrame, SIGNAL(clicked(bool)), this, SLOT(setSystrayVisible(bool)));
	connect(activateAllFrame, SIGNAL(clicked(bool)), this, SLOT(setSystrayActivateAll(bool)));
	connect(activateAllConfirmation, SIGNAL(clicked(bool)), this, SLOT(setSystrayAllConfirmation(bool)));
	connect(tagsConfirmation, SIGNAL(clicked(bool)), this, SLOT(setSystrayTagsConfirmation(bool)));
}

void PrefsPanelDialog::applySampleText()
{
	QString st = sampleTextDefaultText->toPlainText();
// 	qDebug()<< st;
	typotek::getInstance()->setSampleText(st);
	typotek::getInstance()->forwardUpdateView();
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

