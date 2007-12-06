/*******************************************************************
 *
 *  Copyright 2005  David Turner, The FreeType Project (www.freetype.org)
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_STREAM_H
#define HARFBUZZ_STREAM_H

#include "harfbuzz-global.h"

HB_BEGIN_HEADER

typedef struct HB_StreamRec_
{
    HB_Byte*       base;
    HB_UInt        size;
    HB_UInt        pos;
    
    HB_Byte*       cursor;
} HB_StreamRec;


HB_END_HEADER

#endif
