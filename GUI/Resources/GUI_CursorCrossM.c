

#include <stdlib.h>
#include "GUI_Protected.h"

GUI_CONST_STORAGE GUI_BITMAP GUI_BitmapCrossM = {
 21,                   /* XSize */
 21,                   /* YSize */
 6,                    /* BytesPerLine */
 2,                    /* BitsPerPixel */
 GUI_Pixels_CrossM,    /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_CURSOR GUI_CursorCrossM = {
  &GUI_BitmapCrossM, 10, 10
};

/*************************** End of file ****************************/
