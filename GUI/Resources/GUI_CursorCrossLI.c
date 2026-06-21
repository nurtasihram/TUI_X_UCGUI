

#include <stdlib.h>
#include "GUI_Protected.h"

GUI_CONST_STORAGE GUI_BITMAP GUI_BitmapCrossLI = {
 31,                   /* XSize */
 31,                   /* YSize */
 8,                    /* BytesPerLine */
 2,                    /* BitsPerPixel */
 GUI_Pixels_CrossL,    /* Pointer to picture data (indices) */
 &GUI_CursorPalI       /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_CURSOR GUI_CursorCrossLI = {
  &GUI_BitmapCrossLI, 15, 15
};

/*************************** End of file ****************************/
