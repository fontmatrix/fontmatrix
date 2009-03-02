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

#include <QSyntaxHighlighter>
#include <QMap>

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
		void hideEvent( QHideEvent * event ) ;
		
	private:
		QString outBuffer;
		
	public slots:
		void startRunNotice();
		void endRunNotice();
		
	private slots:
		void execScript();
		void showSelectPage(bool);
		void selectScript(QListWidgetItem *);
		void saveScript();
		
	signals:
		void finished();
};

/// ** from scribus source  **
/*! \brief Simple syntax highlighting for Scripter (QTextEdit).
Based on the source of the Sqliteman and Qt4 examples.
but very simplifier. Improved too (of course).
\author Petr Vanek, <petr@yarpen.cz>
 */
class SyntaxHighlighter : public QSyntaxHighlighter
{
	public:
		SyntaxHighlighter(QTextDocument *doc);

	protected:
		void highlightBlock(const QString &text);

		struct HighlightingRule
		{
			QRegExp pattern;
			QTextCharFormat format;
		};
		QVector<HighlightingRule> highlightingRules;
		
		struct SyntaxColors
		{
			SyntaxColors();
			~SyntaxColors();
			QColor errorColor;
			QColor commentColor;
			QColor keywordColor;
			QColor signColor;
			QColor numberColor;
			QColor stringColor;
			QColor textColor;
		};
		SyntaxColors colors;

		QTextCharFormat keywordFormat;
		QTextCharFormat singleLineCommentFormat;
		QTextCharFormat quotationFormat;
		QTextCharFormat numberFormat;
		QTextCharFormat operatorFormat;


};


#endif // FMSCRIPTCONSOLE_H


