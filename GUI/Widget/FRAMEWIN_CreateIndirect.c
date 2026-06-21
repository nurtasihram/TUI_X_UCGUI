
#include "FRAMEWIN.h"

FRAMEWIN_Handle FRAMEWIN_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent,
                                       int x0, int y0, WM_CALLBACK* cb) {
  FRAMEWIN_Handle hObj;
  hObj = FRAMEWIN_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
                           hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName, cb);
  return hObj;
}

