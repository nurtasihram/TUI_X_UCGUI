
#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

const GUI_FONT GUI_UNI_PTR * FRAMEWIN_GetFont(FRAMEWIN_Handle hObj) {
  const GUI_FONT GUI_UNI_PTR * r = NULL;

  if (hObj) {
    FRAMEWIN_Obj* pObj = FRAMEWIN_H2P(hObj);
    r = pObj->Props.pFont;
    FRAMEWIN_Invalidate(hObj);
  }

  return r;
}
int FRAMEWIN_GetTitleHeight(FRAMEWIN_Handle hObj) {
  int r = 0;
  POSITIONS Pos;
  /* Move client window accordingly */
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    FRAMEWIN__CalcPositions(pObj, &Pos);
    r = pObj->Props.TitleHeight;
    if (r == 0) {
      r = Pos.TitleHeight;
    }

  }
  return r;
}
int FRAMEWIN_GetBorderSize(FRAMEWIN_Handle hObj) {
  int r = 0;
  /* Move client window accordingly */
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    r = pObj->Props.BorderSize;

  }
  return r;
}
#else
  void FRAMEWIN_Get_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
