
#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"
#include "MENU.h"

void FRAMEWIN_AddMenu(FRAMEWIN_Handle hObj, WM_HWIN hMenu) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    if (pObj) {
      int TitleHeight, BorderSize, IBorderSize = 0;
      int x0, y0, xSize;
      TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
      BorderSize = pObj->Props.BorderSize;
      if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
        IBorderSize = pObj->Props.IBorderSize;
      }
      x0     = BorderSize;
      y0     = BorderSize + TitleHeight + IBorderSize;
      xSize  = WM__GetWindowSizeX(&pObj->Widget.Win);
      xSize -= BorderSize * 2;
      pObj->hMenu = hMenu;
      if (pObj->cb) {
        MENU_SetOwner(hMenu, pObj->hClient);
      }
      MENU_Attach(hMenu, hObj, x0, y0, xSize, 0, 0);
      WM_SetAnchor(hMenu, WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT);
      FRAMEWIN__UpdatePositions(pObj);
      FRAMEWIN_Invalidate(hObj);
    }

  }
}
#else
  void FRAMEWIN_AddMenu_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
/*************************** End of file ****************************/
