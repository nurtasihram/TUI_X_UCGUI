

#include "GUI_Protected.h"

void GUI_DispStringAt(const char  *s, int x, int y) {

  GUI_Context.DispPosX = x;
  GUI_Context.DispPosY = y;
  GUI_DispString(s);

}


