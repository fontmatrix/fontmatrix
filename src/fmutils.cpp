/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmutils.h"
#include "fontmatrix_debug.h"

#if !defined(_WIN32) && !defined(Q_OS_MAC)
#include <cstdlib>
#include <cxxabi.h>
#include <execinfo.h>

#include <QString>

void printBacktrace(int frames)
{
    auto trace = new void *[frames + 1];
    auto messages = (char **)nullptr;
    int i, trace_size = 0;

    trace_size = backtrace(trace, frames + 1);
    messages = backtrace_symbols(trace, trace_size);
    if (messages) {
        for (i = 1; i < trace_size; ++i) {
            QString msg(QString::fromLocal8Bit(messages[i]));
            int sep1(msg.indexOf(QLatin1String("(")));
            int sep2(msg.indexOf(QLatin1String("+")));
            QString mName(msg.mid(sep1 + 1, sep2 - sep1 - 1));

            QString name;
            if (mName.startsWith(QLatin1String("_Z"))) {
                char *outbuf = nullptr;
                size_t length = 0;
                int status = 0;
                outbuf = abi::__cxa_demangle(mName.trimmed().toLatin1().data(), outbuf, &length, &status);
                name = QString::fromLatin1(outbuf);
                if (0 == status) {
                    //					qDebug()<<"Demangle success["<< length <<"]"<<name;
                    free(outbuf);
                }
                //				else
                //				{
                //					qDebug()<<"Demangle failed ["<<status<<"]["<< mName.trimmed() <<"]";
                //					continue;
                //				}
            } else
                name = mName;
            if (name.isEmpty())
                name = mName;
            QString bts(QStringLiteral("[BT] %1. %2"));
            qCDebug(FONTMATRIX_LOG, "%s", bts.arg(i).arg(name).toUtf8().data());
        }
        free(static_cast<void *>(messages));
    }
    delete[] trace;
}

#endif