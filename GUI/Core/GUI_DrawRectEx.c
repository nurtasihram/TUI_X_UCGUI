

#include "GUI_Protected.h"

void GUI_DrawRectEx(const GUI_RECT* pRect) {
  GUI_DrawRect(pRect->x0, pRect->y0, pRect->x1, pRect->y1);
}

/*************************** End of file ****************************/
