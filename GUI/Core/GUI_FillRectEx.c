

#include "GUI_Protected.h"

void GUI_FillRectEx(const GUI_RECT* pRect) {
  GUI_FillRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);
}

/*************************** End of file ****************************/
