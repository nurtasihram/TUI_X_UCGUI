
#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

static void _ShowHideButtons(FRAMEWIN_Obj* pObj) {
  WM_HWIN hChild;
  WM_Obj* pChild;
  int y0;
  for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
    pChild = WM_H2P(hChild);
    y0 = pChild->Rect.y0 - pObj->Widget.Win.Rect.y0;
    if ((y0 == pObj->Props.BorderSize) && (hChild != pObj->hClient)) {
      if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
        WM_ShowWindow(hChild);
      } else {
        WM_HideWindow(hChild);
      }
    }
  }
}
void FRAMEWIN_SetTitleVis(FRAMEWIN_Handle hObj, int Show) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;
    int State;

    pObj = FRAMEWIN_H2P(hObj);
    State = pObj->Widget.State;
    if (Show) {
      State |= FRAMEWIN_SF_TITLEVIS;
    } else {
      State &= ~FRAMEWIN_SF_TITLEVIS;
    }
    if (pObj->Widget.State != State) {
      pObj->Widget.State = State;
      FRAMEWIN__UpdatePositions(pObj);
      _ShowHideButtons(pObj);
      if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
        if (State & FRAMEWIN_SF_TITLEVIS) {
          WM_ShowWindow(hObj);
        } else {
          WM_HideWindow(hObj);
        }
      }
      FRAMEWIN_Invalidate(hObj);
    }

  }
}
#else
  void FRAMEWIN_SetTitleVis_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
