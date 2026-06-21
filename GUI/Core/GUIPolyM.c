

#include <math.h>
#include "GUI.h"

void GUI_MagnifyPolygon(GUI_POINT* pDest, const GUI_POINT* pSrc, int NumPoints, int Mag) {
  int j;
  for (j=0; j<NumPoints; j++) {
    (pDest+j)->x = (pSrc+j)->x * Mag;
    (pDest+j)->y = (pSrc+j)->y * Mag;
  }
}

/*************************** End of file ****************************/
