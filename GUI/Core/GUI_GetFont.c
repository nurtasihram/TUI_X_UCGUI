

#include "GUI_Protected.h"

const GUI_FONT * GUI_GetFont(void) {
  const GUI_FONT  * r;

  r = GUI_Context.pAFont;

  return r;
}


