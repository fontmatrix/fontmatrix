//
// C++ Interface: fmbrowser
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMBROWSER_H
#define FMBROWSER_H

#include <QProcess>
#include <QUrl>

class FMBrowser : public QProcess
{
		Q_OBJECT

		FMBrowser();
		~FMBrowser();
		static FMBrowser * instance;
	public:
		static FMBrowser * getInstance();
		
		void loadUrl(const QString& urlString);
		void loadUrl(const QUrl& url);

};

#endif // FMBROWSER_H
