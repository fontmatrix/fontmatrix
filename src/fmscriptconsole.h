//
// C++ Interface: fmscriptconsole
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMSCRIPTCONSOLE_H
#define FMSCRIPTCONSOLE_H

#include "ui_scriptconsole.h"

class FMScriptConsole : public QWidget, private Ui::ScriptConsole
{
	Q_OBJECT
	static FMScriptConsole * instance;
	FMScriptConsole();
	~FMScriptConsole(){};
	public:
		static FMScriptConsole* getInstance();
		
		void Out(const QString& s);
		void Err(const QString& s);
	protected:
// 		void closeEvent ( QCloseEvent * event );
		void hideEvent( QHideEvent * event ) ;
	signals:
		void finished();
};


#endif // FMSCRIPTCONSOLE_H


