// PythonQt wrapper implementation

#include "typotek.h"
#include "listdockwidget.h"
#include "fontitem.h"
#include "fmpython_w.h"

#include <QFile>
#include <QDebug>

FMPythonW *FMPythonW::instance = 0;

FMPythonW::FMPythonW()
	:tk ( typotek::getInstance() )
{

}


FMPythonW * FMPythonW::getInstance()
{
	if(!instance)
	{
		instance = new FMPythonW;
		Q_ASSERT(instance);
		instance->init();
		instance->doConnect();
	}
	
	return instance;
}

void FMPythonW::init()
{
	PythonQt::init(PythonQt::RedirectStdOut);
	mainContext = PythonQt::self()->getMainModule();
	mainContext.addObject("Fontmatrix", instance);
}


void FMPythonW::doConnect()
{
// 	connect ( ListDockWidget::getInstance(),
// 	          SIGNAL ( currentChanged() ),
// 	          SIGNAL ( currentChanged() ) );
}


/// "exported" methods

void FMPythonW::nextFace()
{
	qDebug()<<"FMPythonW::nextFace";
	ListDockWidget::getInstance()->fontTree->slotNextFont();
}

void FMPythonW::previousFace()
{
	qDebug()<<"FMPythonW::previousFace";
	ListDockWidget::getInstance()->fontTree->slotPreviousFont();
}

void FMPythonW::nextFamily()
{
	qDebug()<<"FMPythonW::nextFamily";
	ListDockWidget::getInstance()->fontTree->slotNextFamily();
}

void FMPythonW::previousFamily()
{
	qDebug()<<"FMPythonW::previousFamily";
	ListDockWidget::getInstance()->fontTree->slotPreviousFamily();
}

QString FMPythonW::currentFontPath()
{
	if ( !tk->getSelectedFont() )
		return QString();
	return tk->getSelectedFont()->path();
}

QString FMPythonW::currentFontFamily()
{
	if ( !tk->getSelectedFont() )
		return QString();
	return tk->getSelectedFont()->family();
}

QString FMPythonW::currentFontStyle()
{
	if ( !tk->getSelectedFont() )
		return QString();
	return tk->getSelectedFont()->variant();
}

void FMPythonW::run(const QString & pyScript)
{
// 	qDebug()<<"FMPythonW::run"<<pyScript;
	QFile sf(pyScript);
	QString script;
	if(sf.open(QIODevice::ReadOnly))
	{
		script =QString::fromUtf8(sf.readAll());
	}
	else
		qDebug()<<"Error: Cannot open"<<pyScript;
	
	QVariant pyRes(mainContext.evalScript(script, Py_eval_input));
	qDebug()<<"pyres is:"<<pyRes;
	
}


