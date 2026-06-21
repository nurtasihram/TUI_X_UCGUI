

#include "GUI_Protected.h"

int GUI_SetTextAlign(int Align) {
  int r;

  r = GUI_Context.TextAlign;
  GUI_Context.TextAlign = Align;

  return r;
}

/*************************** End of file ****************************/
