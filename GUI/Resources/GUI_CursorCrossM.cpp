#include "GUI_Protected.h"

const uint8_t GUI_Pixels_CrossM[126] = {
________,________,__XXXXXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
XXXXXXXX,XXXXXXXX,XXXXooXX,XXXXXXXX,XXXXXXXX,XX______,
XXoooooo,oooooooo,ooooXXoo,oooooooo,oooooooo,XX______,
XXXXXXXX,XXXXXXXX,XXXXooXX,XXXXXXXX,XXXXXXXX,XX______,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXXXXX,________,________,________,
};

const GUI_BITMAP GUI_BitmapCrossM = {
 21,                   /* XSize */
 21,                   /* YSize */
 6,                    /* BytesPerLine */
 2,                    /* BitsPerPixel */
 GUI_Pixels_CrossM,    /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorCrossM = {
  &GUI_BitmapCrossM, 10, 10
};

const GUI_BITMAP GUI_BitmapCrossMI = {
 21,                   /* XSize */
 21,                   /* YSize */
 6,                    /* BytesPerLine */
 2,                    /* BitsPerPixel */
 GUI_Pixels_CrossM,    /* Pointer to picture data (indices) */
 &GUI_CursorPalI       /* Pointer to palette */
};

const GUI_CURSOR GUI_CursorCrossMI = {
  &GUI_BitmapCrossMI, 10, 10
};
