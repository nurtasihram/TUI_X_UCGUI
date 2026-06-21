
#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

int FRAMEWIN_SetTitleHeight(FRAMEWIN_Handle hObj, int Height) {
  int r = 0;
  if (hObj) {
    FRAMEWIN_Obj* pObj;
    int OldHeight;

    pObj = FRAMEWIN_H2P(hObj);
    r = pObj->Props.TitleHeight;
    if (Height != r) {
      OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
      pObj->Props.TitleHeight = Height;
      FRAMEWIN__UpdatePositions(pObj);
      FRAMEWIN__UpdateButtons(pObj, OldHeight);
      FRAMEWIN_Invalidate(hObj);
    }

  }
  return r;
}
#else
  void FRAMEWIN_SetTitleHeight_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
