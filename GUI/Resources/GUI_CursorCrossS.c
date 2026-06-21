

#include <stdlib.h>
#include "GUI_Protected.h"

GUI_CONST_STORAGE GUI_BITMAP GUI_BitmapCrossS = {
 11,                   /* XSize */
 11,                   /* YSize */
 3,                    /* BytesPerLine */
 2,                    /* BitsPerPixel */
 GUI_Pixels_CrossS,    /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_CURSOR GUI_CursorCrossS = {
  &GUI_BitmapCrossS, 5, 5
};

/*************************** End of file ****************************/

