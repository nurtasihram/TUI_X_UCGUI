

#include <stdlib.h>
#include "GUI_Protected.h"

static GUI_CONST_STORAGE RGB_COLOR _aColor[] = {
  0x0000FF, 0xFFFFFF, 0x000000
};

GUI_CONST_STORAGE GUI_LOGPALETTE GUI_CursorPalI = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_aColor[0]
};

/*************************** End of file ****************************/
