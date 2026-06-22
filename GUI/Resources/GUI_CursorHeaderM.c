#include "GUI_Protected.h"

const uint8_t GUI_PixelsHeaderM[5 * 17] = {
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
____XX__,______XX,ooXX____,____XX__,________,
__XXooXX,XXXXXXXX,ooXXXXXX,XXXXooXX,________,
XXoooooo,oooooooo,oooooooo,oooooooo,XX______,
__XXooXX,XXXXXXXX,ooXXXXXX,XXXXooXX,________,
____XX__,______XX,ooXX____,____XX__,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
________,______XX,ooXX____,________,________,
};

static const GUI_BITMAP _BitmapHeaderM = {
 17,                  /* XSize */
 17,                  /* YSize */
 5,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_PixelsHeaderM,      /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorHeaderM = {
  &_BitmapHeaderM, 8, 8
};

static const GUI_BITMAP _Bitmap = {
 17,                  /* XSize */
 17,                  /* YSize */
 5,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_PixelsHeaderM,   /* Pointer to picture data (indices) */
 &GUI_CursorPalI      /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorHeaderMI = {
  &_Bitmap, 8, 8
};
