

#include <stdlib.h>
#include "GUI_Protected.h"

GUI_CONST_STORAGE GUI_BITMAP GUI_BitmapCrossSI = {
 11,                  /* XSize */
 11,                  /* YSize */
 3,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_Pixels_CrossS,   /* Pointer to picture data (indices) */
 &GUI_CursorPalI      /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_CURSOR GUI_CursorCrossSI = {
  &GUI_BitmapCrossSI, 5, 5
};

/*************************** End of file ****************************/
