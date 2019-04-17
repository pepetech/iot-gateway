#ifndef __FONT_H_
#define __FONT_H_

#include "stdint.h"

/// Font data stored per glyph
typedef struct
{
	uint16_t    bitmapOffset;     ///< Pointer into GFXfont->bitmap
	uint8_t     width;            ///< Bitmap dimensions in pixels
    uint8_t     height;           ///< Bitmap dimensions in pixels
	uint8_t     xAdvance;         ///< Distance to advance cursor (x axis)
	int8_t      xOffset;          ///< X dist from cursor pos to UL corner
    int8_t      yOffset;          ///< Y dist from cursor pos to UL corner
} glyph_t;

/// Data stored for font as a whole
typedef struct
{
	uint8_t     *bitmap;      ///< Glyph bitmaps, concatenated
	glyph_t     *glyph;       ///< Glyph array
	uint8_t     first;       ///< ASCII extents (first char)
    uint8_t     last;        ///< ASCII extents (last char)
	uint8_t     yAdvance;    ///< Newline distance (y axis)
} font_t;

#endif // __FONT_H_