#include "GUI_Protected.h"

const uint8_t GUI_Pixels_ArrowM[60] = {
XX______,________,________,
XXXX____,________,________,
XXooXX__,________,________,
XXooooXX,________,________,
XXoooooo,XX______,________,
XXoooooo,ooXX____,________,
XXoooooo,ooooXX__,________,
XXoooooo,ooooooXX,________,
XXoooooo,oooooooo,XX______,
XXoooooo,oooooooo,ooXX____,
XXoooooo,oooooooo,ooooXX__,
XXoooooo,ooooooXX,XXXXXXXX,
XXooooXX,ooooooXX,________,
XXooXX__,XXoooooo,XX______,
XXXX____,XXoooooo,XX______,
XX______,__XXoooo,ooXX____,
________,__XXoooo,ooXX____,
________,____XXoo,ooooXX__,
________,____XXoo,ooooXX__,
________,______XX,XXXX____,
};

const GUI_BITMAP GUI_BitmapArrowM = {
 12,                  /* XSize */
 20,                  /* YSize */
 3,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_ArrowM,   /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorArrowM = {
  &GUI_BitmapArrowM, 0, 0
};

const GUI_BITMAP GUI_BitmapArrowMI = {
 12,                  /* XSize */
 20,                  /* YSize */
 3,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_ArrowM,   /* Pointer to picture data (indices) */
 &GUI_CursorPalI      /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorArrowMI = {
  &GUI_BitmapArrowMI, 0, 0
};
