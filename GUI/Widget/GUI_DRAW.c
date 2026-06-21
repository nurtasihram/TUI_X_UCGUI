
#include "GUI.h"
#include "GUI_Protected.h"
#include "WIDGET.h"

void GUI_DRAW__Draw(GUI_DRAW_HANDLE hDrawObj, int x, int y) {
  if (hDrawObj) {
    GUI_DRAW * pDrawObj;
    pDrawObj = (GUI_DRAW *)GUI_ALLOC_h2p(hDrawObj);
    pDrawObj->pConsts->pfDraw(pDrawObj, x, y);
  }
}
int GUI_DRAW__GetXSize(GUI_DRAW_HANDLE hDrawObj) {
  if (hDrawObj) {
    GUI_DRAW * pDrawObj;
    pDrawObj = (GUI_DRAW *)GUI_ALLOC_h2p(hDrawObj);
    return pDrawObj->pConsts->pfGetXSize(pDrawObj);
  }
  return 0;
}
int GUI_DRAW__GetYSize(GUI_DRAW_HANDLE hDrawObj) {
  if (hDrawObj) {
    GUI_DRAW * pDrawObj;
    pDrawObj = (GUI_DRAW *)GUI_ALLOC_h2p(hDrawObj);
    return pDrawObj->pConsts->pfGetYSize(pDrawObj);
  }
  return 0;
}

/*************************** End of file ****************************/
