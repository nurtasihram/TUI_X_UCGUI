
#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"
#include "WIDGET.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

static void _InvalidateButton(FRAMEWIN_Obj* pObj, int Id) {
  WM_HWIN hChild;
  WM_Obj* pChild;
  for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
    pChild = WM_H2P(hChild);
    if (WM_GetId(hChild) == Id) {
      WM_InvalidateWindow(hChild);
    }
  }
}
static void _RestoreMinimized(FRAMEWIN_Handle hObj, FRAMEWIN_Obj* pObj) {
  /* When window was minimized, restore it */
  if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
    int OldHeight = 1 + pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0;
    int NewHeight = 1 + pObj->rRestore.y1 - pObj->rRestore.y0;
    WM_ResizeWindow(hObj, 0, NewHeight - OldHeight);
    WM_ShowWindow(pObj->hClient);
    WM_ShowWindow(pObj->hMenu);
    FRAMEWIN__UpdatePositions(pObj);
    pObj->Flags &= ~FRAMEWIN_SF_MINIMIZED;
    _InvalidateButton(pObj, GUI_ID_MINIMIZE);
  }
}
static void _RestoreMaximized(FRAMEWIN_Handle hObj, FRAMEWIN_Obj* pObj) {
  /* When window was maximized, restore it */
  if (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) {
    GUI_RECT r = pObj->rRestore;
    WM_MoveTo(hObj, r.x0, r.y0);
    WM_SetSize(hObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
    FRAMEWIN__UpdatePositions(pObj);
    pObj->Flags &= ~FRAMEWIN_SF_MAXIMIZED;
    _InvalidateButton(pObj, GUI_ID_MAXIMIZE);
  }
}
static void _MinimizeFramewin(FRAMEWIN_Handle hObj, FRAMEWIN_Obj* pObj) {
  _RestoreMaximized(hObj, pObj);
  /* When window is not minimized, minimize it */
  if ((pObj->Flags & FRAMEWIN_SF_MINIMIZED) == 0) {
    int OldHeight = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
    int NewHeight = FRAMEWIN__CalcTitleHeight(pObj) + pObj->Widget.pEffect->EffectSize * 2 + 2;
    pObj->rRestore = pObj->Widget.Win.Rect;
    WM_HideWindow(pObj->hClient);
    WM_HideWindow(pObj->hMenu);
    WM_ResizeWindow(hObj, 0, NewHeight - OldHeight);
    FRAMEWIN__UpdatePositions(pObj);
    pObj->Flags |= FRAMEWIN_SF_MINIMIZED;
    _InvalidateButton(pObj, GUI_ID_MINIMIZE);
  }
}
static void _MaximizeFramewin(FRAMEWIN_Handle hObj, FRAMEWIN_Obj* pObj) {
  _RestoreMinimized(hObj, pObj);
  /* When window is not maximized, maximize it */
  if ((pObj->Flags & FRAMEWIN_SF_MAXIMIZED) == 0) {
    WM_HWIN hParent = pObj->Widget.Win.hParent;
    WM_Obj* pParent = WM_H2P(hParent);
    GUI_RECT r = pParent->Rect;
    if (pParent->hParent == 0) {
      r.x1 = LCD_GetXSize();
      r.y1 = LCD_GetYSize();
    }
    pObj->rRestore = pObj->Widget.Win.Rect;
    WM_MoveTo(hObj, r.x0, r.y0);
    WM_SetSize(hObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
    FRAMEWIN__UpdatePositions(pObj);
    pObj->Flags |= FRAMEWIN_SF_MAXIMIZED;
    _InvalidateButton(pObj, GUI_ID_MAXIMIZE);
  }
}
void FRAMEWIN_Minimize(FRAMEWIN_Handle hObj) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    _MinimizeFramewin(hObj, pObj);

  }
}
void FRAMEWIN_Maximize(FRAMEWIN_Handle hObj) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    _MaximizeFramewin(hObj, pObj);

  }
}
void FRAMEWIN_Restore(FRAMEWIN_Handle hObj) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    _RestoreMinimized(hObj, pObj);
    _RestoreMaximized(hObj, pObj);

  }
}
#else
  void FRAMEWIN_MinMaxRest_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
