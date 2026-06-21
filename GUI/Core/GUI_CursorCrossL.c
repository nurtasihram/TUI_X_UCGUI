

#include <stdlib.h>
#include "GUI_Protected.h"

GUI_CONST_STORAGE GUI_BITMAP GUI_BitmapCrossL = {
 31,                  /* XSize */
 31,                  /* YSize */
 8,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_CrossL,   /* Pointer to picture data (indices) */
 &GUI_CursorPal      /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_CURSOR GUI_CursorCrossL = {
  &GUI_BitmapCrossL, 15, 15
};

/*************************** End of file ****************************/
