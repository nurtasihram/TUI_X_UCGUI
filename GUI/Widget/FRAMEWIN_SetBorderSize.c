
#include "FRAMEWIN_Private.h"
#include "WIDGET.h"

void FRAMEWIN_SetBorderSize(FRAMEWIN_Handle hObj, unsigned Size) {

  if (hObj) {
    GUI_RECT r;
    WM_Obj * pChild;
    int Diff, OldSize, OldHeight;
    WM_HWIN hChild;
    FRAMEWIN_Obj* pObj = FRAMEWIN_H2P(hObj);
    OldHeight          = FRAMEWIN__CalcTitleHeight(pObj);
    OldSize            = pObj->Props.BorderSize;
    Diff               = Size - OldSize;
    for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
      pChild = WM_H2P(hChild);
      r = pChild->Rect;
      GUI_MoveRect(&r, -pObj->Widget.Win.Rect.x0, -pObj->Widget.Win.Rect.y0);
      if ((r.y0 == pObj->Props.BorderSize) && ((r.y1 - r.y0 + 1) == OldHeight)) {
        if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
          WM_MoveWindow(hChild, -Diff, Diff);
        } else {
          WM_MoveWindow(hChild, Diff, Diff);
        }
      }
    }
    pObj->Props.BorderSize   = Size;
    FRAMEWIN__UpdatePositions(pObj);
    FRAMEWIN_Invalidate(hObj);
  }

}
 /* GUI_WINSUPPORT */
