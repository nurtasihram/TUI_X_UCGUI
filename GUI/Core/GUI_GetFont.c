

#include "GUI_Protected.h"

const GUI_FONT GUI_UNI_PTR* GUI_GetFont(void) {
  const GUI_FONT GUI_UNI_PTR * r;

  r = GUI_Context.pAFont;

  return r;
}

/*************************** End of file ****************************/
