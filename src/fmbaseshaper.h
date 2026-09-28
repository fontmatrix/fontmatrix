/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMBASESHAPER_H
#define FMBASESHAPER_H

#include "fmotf.h"
#include <QString>
#include <QStringList>

/// There’s not too much to say, just subclass it and provide a method
/// which can deal with an OTF handle and a script to produce a glyphs string
/// A piece of cake :-)
class FMBaseShaper
{
public:
    FMBaseShaper(FMOtf *o, QString s);
    virtual ~FMBaseShaper();
    virtual GlyphList doShape(const QString &aString) = 0;

protected:
    FMOtf *otf = nullptr;
    QString script;

private:
    Q_DISABLE_COPY(FMBaseShaper)
};

class FMShaperFactory
{
public:
    enum SHAPER_TYPE {
        FONTMATRIX = 1, // our dear own shaper
        HARFBUZZ,
        NOT_A_SHAPER
    };

    FMShaperFactory(FMOtf *otf, QString script, SHAPER_TYPE st = FONTMATRIX);
    ~FMShaperFactory();

    GlyphList doShape(const QString &aString);
    // 		void resetShaperType ( SHAPER_TYPE st = FONTMATRIX );

    // If you ever think to create your own "shaping strategy",
    // start by adding an entry here, half of the work :-)

private:
    SHAPER_TYPE shaperType;
    FMOtf *otf = nullptr;
    QString script;

    FMBaseShaper *shaperImpl = nullptr;

    Q_DISABLE_COPY(FMShaperFactory)
};

#endif
