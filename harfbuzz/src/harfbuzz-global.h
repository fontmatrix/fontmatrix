/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *  Copyright 2007  Behdad Esfahbod
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_GLOBAL_H
#define HARFBUZZ_GLOBAL_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#define HB_BEGIN_HEADER  extern "C" {
#define HB_END_HEADER  }
#else
#define HB_BEGIN_HEADER  /* nothing */
#define HB_END_HEADER  /* nothing */
#endif

HB_BEGIN_HEADER

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define HB_MAKE_TAG( _x1, _x2, _x3, _x4 ) \
          ( ( (HB_UInt)_x1 << 24 ) |     \
            ( (HB_UInt)_x2 << 16 ) |     \
            ( (HB_UInt)_x3 <<  8 ) |     \
              (HB_UInt)_x4         )

typedef char hb_int8;
typedef unsigned char hb_uint8;
typedef short hb_int16;
typedef unsigned short hb_uint16;
typedef int hb_int32;
typedef unsigned int hb_uint32;

typedef hb_uint8 HB_Bool;

typedef hb_uint8 HB_Byte;
typedef hb_uint16 HB_UShort;
typedef hb_uint32 HB_UInt;
typedef hb_int8 HB_Char;
typedef hb_int16 HB_Short;
typedef hb_int32 HB_Int;

typedef hb_uint16 HB_UChar16;
typedef hb_uint32 HB_UChar32;
typedef hb_uint32 HB_Glyph;
typedef hb_int32 HB_Fixed; /* 26.6 */

#define HB_FIXED_CONSTANT(v) ((v) * 64)
#define HB_FIXED_ROUND(v) (((v)+32) & -64)

typedef hb_int32 HB_16Dot16; /* 16.16 */

typedef void * HB_Pointer;
typedef hb_uint32 HB_Tag;

typedef enum {
  /* no error */
  HB_Err_Ok                           = 0x0000,
  HB_Err_Not_Covered                  = 0xFFFF,

  /* _hb_err() is called whenever returning the following errors,
   * and in a couple places for HB_Err_Not_Covered too. */

  /* programmer error */
  HB_Err_Invalid_Argument             = 0x1A66,

  /* font error */
  HB_Err_Invalid_SubTable_Format      = 0x157F,
  HB_Err_Invalid_SubTable             = 0x1570,
  HB_Err_Read_Error                   = 0x6EAD,

  /* system error */
  HB_Err_Out_Of_Memory                = 0xDEAD
} HB_Error;

typedef struct {
    HB_Fixed x;
    HB_Fixed y;
} HB_FixedPoint;

typedef struct HB_Font_ *HB_Font;
typedef struct HB_StreamRec_ *HB_Stream;
typedef struct HB_FaceRec_ *HB_Face;

HB_END_HEADER

#endif
