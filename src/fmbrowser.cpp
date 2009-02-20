//
// C++ Implementation: fmbrowser
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmbrowser.h"

#include <QSettings>

FMBrowser * FMBrowser::instance = 0;
FMBrowser::FMBrowser()
	:QProcess()
{
	
}

FMBrowser::~ FMBrowser()
{
}

FMBrowser * FMBrowser::getInstance()
{
}

void FMBrowser::loadUrl(const QString & urlString)
{
}

void FMBrowser::loadUrl(const QUrl & url)
{
}
