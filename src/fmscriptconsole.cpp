//
// C++ Implementation: fmscriptconsole
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmscriptconsole.h"

#include <QCloseEvent>

FMScriptConsole * FMScriptConsole::instance = 0;
FMScriptConsole::FMScriptConsole()
	:QWidget(0)
{
	setupUi(this);
}

FMScriptConsole * FMScriptConsole::getInstance()
{
	if(!instance)
	{
		instance = new FMScriptConsole;
		Q_ASSERT(instance);
	}
	return instance;
}

void FMScriptConsole::Out(const QString & s)
{
	QString t(stdOut->toPlainText());
	t.append(s);
	stdOut->setText(t);
}

void FMScriptConsole::Err(const QString & s)
{
	QString t(stdErr->toPlainText());
	t.append(s);
	stdErr->setText(t);
}

// void FMScriptConsole::closeEvent(QCloseEvent * event)
// {
// 	qDebug("FMScriptConsole::closeEvent");
// 	event->accept();
// 	emit finished();
// }

void FMScriptConsole::hideEvent(QHideEvent * event)
{
	emit finished();
}
