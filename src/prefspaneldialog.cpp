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

PrefsPanelDialog::PrefsPanelDialog(QWidget *parent)
 : QDialog(parent)
{
	setupUi(this);
	QString st = typotek::getInstance()->sampleText();
// 	qDebug()<< st;
	sampleTextDefaultText->setPlainText(st);
	doConnect();
}


PrefsPanelDialog::~PrefsPanelDialog()
{
}

void PrefsPanelDialog::doConnect()
{
	connect(applySampleTextButton,SIGNAL(released()),this,SLOT(applySampleText()));
}

void PrefsPanelDialog::applySampleText()
{
	QString st = sampleTextDefaultText->toPlainText();
// 	qDebug()<< st;
	typotek::getInstance()->setSampleText(st);
	typotek::getInstance()->forwardUpdateView();
}


