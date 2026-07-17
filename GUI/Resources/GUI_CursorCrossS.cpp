#include "GUI_Protected.h"

const uint8_t GUI_Pixels_CrossS[33] = {
________,XXXXXX__,________,
________,XXooXX__,________,
________,XXooXX__,________,
________,XXooXX__,________,
XXXXXXXX,XXooXXXX,XXXXXX__,
XXoooooo,ooXXoooo,ooooXX__,
XXXXXXXX,XXooXXXX,XXXXXX__,
________,XXooXX__,________,
________,XXooXX__,________,
________,XXooXX__,________,
________,XXXXXX__,________,
};

const GUI_BITMAP GUI_BitmapCrossS = {
 11,                   /* XSize */
 11,                   /* YSize */
 3,                    /* BytesPerLine */
 2,                    /* BitsPerPixel */
 GUI_Pixels_CrossS,    /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorCrossS = {
  &GUI_BitmapCrossS, 5, 5
};

const GUI_BITMAP GUI_BitmapCrossSI = {
 11,                  /* XSize */
 11,                  /* YSize */
 3,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_CrossS,   /* Pointer to picture data (indices) */
 &GUI_CursorPalI      /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorCrossSI = {
  &GUI_BitmapCrossSI, 5, 5
};
