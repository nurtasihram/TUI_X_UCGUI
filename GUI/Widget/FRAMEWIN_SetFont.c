
#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

void FRAMEWIN_SetFont(FRAMEWIN_Handle hObj, const GUI_FONT GUI_UNI_PTR * pFont) {

  if (hObj) {
    FRAMEWIN_Obj* pObj = FRAMEWIN_H2P(hObj);
    int OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
    pObj->Props.pFont = pFont;
    FRAMEWIN__UpdatePositions(pObj);
    FRAMEWIN__UpdateButtons(pObj, OldHeight);
    FRAMEWIN_Invalidate(hObj);
  }

}
#else
  void FRAMEWIN_SetFont_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
