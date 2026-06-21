

#include "GUI_Protected.h"

static GUI_CONST_STORAGE GUI_BITMAP _BitmapHeaderM = {
 17,                  /* XSize */
 17,                  /* YSize */
 5,                   /* BytesPerLine */
 2,                   /* BitsPerPixel */
 GUI_PixelsHeaderM,      /* Pointer to picture data (indices) */
 &GUI_CursorPal       /* Pointer to palette */
};

GUI_CONST_STORAGE GUI_CURSOR GUI_CursorHeaderM = {
  &_BitmapHeaderM, 8, 8
};

/*************************** End of file ****************************/
