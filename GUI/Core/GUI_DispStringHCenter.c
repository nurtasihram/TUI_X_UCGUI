

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include "GUI_Protected.h"

void GUI_DispStringHCenterAt(const char GUI_UNI_PTR *s, int x, int y) {
  int Align;
  Align = GUI_SetTextAlign((GUI_Context.TextAlign & ~GUI_TA_LEFT) | GUI_TA_CENTER);
  GUI_DispStringAt(s, x, y);
  GUI_SetTextAlign(Align);
}


