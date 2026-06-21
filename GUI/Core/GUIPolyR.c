

#include <math.h>
#include "GUI.h"

void GUI_RotatePolygon(GUI_POINT* pDest, const GUI_POINT* pSrc, int NumPoints, float Angle) {
  int j;
  float fcos = cos(Angle);
  float fsin = sin(Angle);
  for (j=0; j<NumPoints; j++) {
    int x = (pSrc+j)->x;
    int y = (pSrc+j)->y;
    (pDest+j)->x =  x * fcos + y * fsin;
    (pDest+j)->y = -x * fsin + y * fcos;
  }
}

/*************************** End of file ****************************/
