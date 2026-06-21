

#include "GUI_Protected.h"

int GUI_SetTextMode(int Mode) {
  int r;

  r = GUI_Context.TextMode;
  GUI_Context.TextMode = Mode;

  return r;
}

/*************************** End of file ****************************/
