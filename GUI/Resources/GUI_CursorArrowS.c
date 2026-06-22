#include "GUI_Protected.h"

const uint8_t GUI_Pixels_ArrowS[45] = {
XX______,________,________,
XXXX____,________,________,
XXooXX__,________,________,
XXooooXX,________,________,
XXoooooo,XX______,________,
XXoooooo,ooXX____,________,
XXoooooo,ooooXX__,________,
XXoooooo,ooooooXX,________,
XXoooooo,ooXXXXXX,XX______,
XXooXXoo,ooXX____,________,
XXXX__XX,ooooXX__,________,
XX____XX,ooooXX__,________,
________,XXooooXX,________,
________,XXooooXX,________,
________,__XXXX__,________,
};

const GUI_BITMAP GUI_BitmapArrowS = {
  9,                  /* XSize */
 15,                  /* YSize */
 3,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_ArrowS,   /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorArrowS = {
  &GUI_BitmapArrowS, 0, 0
};

const GUI_BITMAP GUI_BitmapArrowSI = {
  9,                  /* XSize */
 15,                  /* YSize */
 3,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_ArrowS,   /* Pointer to picture data (indices) */
 &GUI_CursorPalI      /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorArrowSI = {
  &GUI_BitmapArrowSI, 0, 0
};
