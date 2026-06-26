

#include "GUI_Protected.h"

void GUI_DispStringAtCEOL(const char  *s, int x, int y) {
  GUI_DispStringAt(s,x,y);
  GUI_DispCEOL();
}


