

#include "GUI_Protected.h"

void GUI_ClearRectEx(const GUI_RECT* pRect) {
  GUI_ClearRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);
}

/*************************** End of file ****************************/
