

#include "GUI_Protected.h"

void GUI__ReduceRect(GUI_RECT* pDest, const GUI_RECT *pRect, int Dist) {
  pDest->x0 = pRect->x0 + Dist;
  pDest->x1 = pRect->x1 - Dist;
  pDest->y0 = pRect->y0 + Dist;
  pDest->y1 = pRect->y1 - Dist;
}

/*************************** End of file ****************************/
