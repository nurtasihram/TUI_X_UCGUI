#include <stdlib.h>
#include "GUI_Protected.h"

static const RGB_COLOR _aColor[] = {
  RGB_RED, RGB_BLACK, RGB_WHITE
};

const GUI_LOGPALETTE GUI_CursorPal = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_aColor[0]
};

static const RGB_COLOR _aColorI[] = {
  RGB_RED, RGB_WHITE, RGB_BLACK
};

const GUI_LOGPALETTE GUI_CursorPalI = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_aColorI[0]
};

