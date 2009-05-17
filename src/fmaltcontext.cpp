//
// C++ Implementation: fmaltcontext
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmaltcontext.h"

#include <QDebug>

FMAltContextLib * FMAltContextLib::instance = 0;
FMAltContextLib::FMAltContextLib()
{
	cmap.clear();
	current.clear();
}

FMAltContextLib::~ FMAltContextLib()
{
	foreach ( FMAltContext* actx, cmap.values() )
	{
		delete actx;
	}
}

FMAltContextLib * FMAltContextLib::that()
{
	if ( !instance )
	{
		instance = new FMAltContextLib();
		Q_ASSERT ( instance );
	}
	return instance;
}


FMAltContext * FMAltContextLib::SetCurrentContext ( const QString & tid, const QString& font )
{
	QString cid(tid + font);
	if ( !that()->cmap.contains ( cid ) )
	{
		qDebug()<<"CREATE context"<<cid;
		that()->cmap[cid] = new FMAltContext();
		that()->cmap[cid]->fontID = font;
		that()->cmap[cid]->textID = tid;
	}
	that()->current = cid;
	qDebug()<<"CTX"<<cid;

	emit that()->contextChanged();

	return that()->cmap[cid];

}

FMAltContext * FMAltContextLib::GetCurrentContext() 
{
	if ( that()->cmap.contains ( that()->current ) )
	{
		return that()->cmap[that()->current];
	}
	return 0;
}


void FMAltContextLib::GetConnected(const QObject * receiver, const char * method)
{
	connect(that(), SIGNAL(contextChanged()), receiver, method);
}
