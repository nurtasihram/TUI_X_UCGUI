

#include "GUI_Protected.h"

void GUI_DispStringAt(const char GUI_UNI_PTR *s, int x, int y) {

  GUI_Context.DispPosX = x;
  GUI_Context.DispPosY = y;
  GUI_DispString(s);

}

/*************************** End of file ****************************/
