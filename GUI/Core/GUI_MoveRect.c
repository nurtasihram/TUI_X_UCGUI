

#include "GUI_Protected.h"

void GUI_MoveRect(GUI_RECT *pRect, int dx, int dy) {
  if (pRect) {
    pRect->x0 += dx;
    pRect->x1 += dx;
    pRect->y0 += dy;
    pRect->y1 += dy;
  }
}


