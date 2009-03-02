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
#include "fmpython_w.h"
#include "fmpaths.h"

#include <QCloseEvent>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

FMScriptConsole * FMScriptConsole::instance = 0;
FMScriptConsole::FMScriptConsole()
	:QWidget(0)
{
	setupUi(this);
	new SyntaxHighlighter(input->document());
	
	connect(execButton, SIGNAL(clicked()), this, SLOT(execScript()));
	connect(loadButton, SIGNAL (toggled(bool)), this, SLOT(showSelectPage(bool)));
	connect(saveButton,  SIGNAL(clicked()), this, SLOT(saveScript()));
	
	connect(scriptsList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(selectScript(QListWidgetItem*)));
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
	stdOut->textCursor().movePosition(QTextCursor::End);
}

void FMScriptConsole::Err(const QString & s)
{
	QString t(stdErr->toPlainText());
	t.append(s);
	stdErr->setText(t);
	stdErr->textCursor().movePosition(QTextCursor::End);
}

void FMScriptConsole::hideEvent(QHideEvent * event)
{
	emit finished();
}

void FMScriptConsole::execScript()
{
	QString sc(input->toPlainText());
	FMPythonW::getInstance()->runString(sc);
}

void FMScriptConsole::showSelectPage(bool cho)
{
	if(cho)
	{
		scriptsList->clear();
		
		QDir Rdir(FMPaths::ResourcesDir() + "Scripts");
		Rdir.setNameFilters(QStringList()<<"*.py");
		QStringList Rfiles(Rdir.entryList());
		scriptsList->addItems(Rfiles);
		
		QDir dir(FMPaths::ScriptsDir());
		dir.setNameFilters(QStringList()<<"*.py");
		QStringList files(dir.entryList());
		scriptsList->addItems(files);
		
		stackInput->setCurrentIndex(1);
	}
	else
		stackInput->setCurrentIndex(0);
}

void FMScriptConsole::selectScript(QListWidgetItem * item)
{
	if(!item)
		return;
	// first personal scripts
	QFile f(FMPaths::ScriptsDir()+item->text());
	if(f.open(QIODevice::ReadOnly))
	{
		QByteArray a(f.readAll());
		input->setPlainText(a);
		loadButton->setChecked(false);
		return;
	}
	// then system ones
	QFile rf(FMPaths::ResourcesDir() + "Scripts"+QDir::separator()+item->text());
	if(rf.open(QIODevice::ReadOnly))
	{
		QByteArray a(rf.readAll());
		input->setPlainText(a);
		loadButton->setChecked(false);
		return;
	}
	
}

void FMScriptConsole::saveScript()
{
	QString f(QInputDialog::getText(this,"Fontmatrix", tr("A name for the script to save:")));
	if(f.isEmpty())
		return;
	if(!f.endsWith(".py"))
		f += ".py";
	QDir dir(FMPaths::ScriptsDir());
	dir.setNameFilters(QStringList()<<"*.py");
	QStringList files(dir.entryList());
	if(files.contains(f))
	{
		QMessageBox::warning(this, "Fontmatrix", tr("The file %1 already exists").arg(f));
		return;
	}
	QFile file(FMPaths::ScriptsDir() + f);
	if(file.open(QIODevice::WriteOnly))
	{
		file.write(input->toPlainText().toUtf8());
	}
}


/*
 * Syntax highlighting
 */
SyntaxHighlighter::SyntaxHighlighter(QTextDocument *doc) : QSyntaxHighlighter(doc)
{
	// Reserved keywords in Python 2.4
	QStringList keywords;
	HighlightingRule rule;

	keywords << "and" << "assert" << "break" << "class" << "continue" << "def"
			<< "del" << "elif" << "else" << "except" << "exec" << "finally"
			<< "for" << "from" << "global" << "if" << "import" << "in"
			<< "is" << "lambda" << "not" << "or" << "pass" << "print" << "raise"
			<< "return" << "try" << "while" << "yield";

	keywordFormat.setForeground(colors.keywordColor);
	keywordFormat.setFontWeight(QFont::Bold);
	singleLineCommentFormat.setForeground(colors.commentColor);
	singleLineCommentFormat.setFontItalic(true);
	quotationFormat.setForeground(colors.stringColor);
	numberFormat.setForeground(colors.numberColor);
	operatorFormat.setForeground(colors.signColor);

	foreach (QString kw, keywords)
	{
		rule.pattern = QRegExp("\\b" + kw + "\\b", Qt::CaseInsensitive);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	rule.pattern = QRegExp("#[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegExp("\'.*\'");
	rule.pattern.setMinimal(true);
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegExp("\".*\"");
	rule.pattern.setMinimal(true);
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegExp("\\b\\d+\\b");
	rule.pattern.setMinimal(true);
	rule.format = numberFormat;
	highlightingRules.append(rule);

	rule.pattern = QRegExp("[\\\\|\\<|\\>|\\=|\\!|\\+|\\-|\\*|\\/|\\%]+");
	rule.pattern.setMinimal(true);
	rule.format = operatorFormat;
	highlightingRules.append(rule);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
	foreach (HighlightingRule rule, highlightingRules)
	{
		QRegExp expression(rule.pattern);
		int index = text.indexOf(expression);
		while (index >= 0)
		{
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = text.indexOf(expression, index + length);
		}
	}
	setCurrentBlockState(0);

	// multiline strings handling
	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = text.indexOf("\"\"\"");

	while (startIndex >= 0)
	{
		int endIndex = text.indexOf("\"\"\"", startIndex);
		int commentLength;

		if (endIndex == -1)
		{
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else
		{
			commentLength = endIndex - startIndex + 3;//commentEndExpression.matchedLength();
		}
		setFormat(startIndex, commentLength, quotationFormat);
		startIndex = text.indexOf("\"\"\"", startIndex + commentLength);
	}
}

SyntaxHighlighter::SyntaxColors::SyntaxColors()
{
	QSettings settings;
	errorColor 	= 	QColor(settings.value("Python/SyntaxError", "#aa0000").toString());
	commentColor 	= 	QColor(settings.value("Python/SyntaxComment", "#A0A0A0").toString());
	keywordColor 	= 	QColor(settings.value("Python/SyntaxKeyword", "#00007f").toString());
	signColor 	= 	QColor(settings.value("Python/SyntaxSign", "#aa00ff").toString());
	numberColor 	= 	QColor(settings.value("Python/SyntaxNumber", "#ffaa00").toString());
	stringColor 	= 	QColor(settings.value("Python/SyntaxString", "#005500").toString());
	textColor 	= 	QColor(settings.value("Python/SyntaxText", "#000000").toString());
}

SyntaxHighlighter::SyntaxColors::~SyntaxColors()
{
	QSettings settings;
	settings.setValue("Python/SyntaxError",errorColor.name());
	settings.setValue("Python/SyntaxComment",commentColor.name());
	settings.setValue("Python/SyntaxKeyword",keywordColor.name());
	settings.setValue("Python/SyntaxSign",signColor.name());
	settings.setValue("Python/SyntaxNumber",numberColor.name());
	settings.setValue("Python/SyntaxString",stringColor.name());
	settings.setValue("Python/SyntaxText",textColor.name());
	
}



