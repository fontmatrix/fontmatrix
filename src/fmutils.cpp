//
// C++ Implementation: fmutils
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmutils.h"

#if !defined(_WIN32) && !defined(Q_OS_MAC)
#include <execinfo.h>
#include <cxxabi.h>
#include <cstdlib>

#include <QString>

void printBacktrace ( int frames )
{
	void ** trace = new void*[frames + 1];
	char **messages = ( char ** ) NULL;
	int i, trace_size = 0;

	trace_size = backtrace ( trace, frames + 1 );
	messages = backtrace_symbols ( trace, trace_size );
	if ( messages )
	{
		for ( i=1; i < trace_size; ++i )
		{
			QString msg ( messages[i] );
			int sep1 ( msg.indexOf ( "(" ) );
			int sep2 ( msg.indexOf ( "+" ) );
			QString mName (	msg.mid ( sep1 + 1,sep2-sep1 -1 ) );

			QString name;
			if ( mName.startsWith ( "_Z" ) )
			{
				char* outbuf = 0;
				size_t length = 0;
				int status = 0;
				outbuf = abi::__cxa_demangle ( mName.trimmed().toLatin1().data(), outbuf, &length, &status );
				name = QString::fromLatin1 ( outbuf );
				if ( 0 == status )
				{
//					qDebug()<<"Demangle success["<< length <<"]"<<name;
					free ( outbuf );
				}
//				else
//				{
//					qDebug()<<"Demangle failed ["<<status<<"]["<< mName.trimmed() <<"]";
//					continue;
//				}
			}
			else
				name = mName;
			if ( name.isEmpty() )
				name = mName;
			QString bts ( "[BT] %1. %2" );
			qDebug ( "%s", bts.arg ( i ).arg ( name ).toUtf8().data() );
		}
		free ( messages );
	}
	delete[] trace;
}


#endif