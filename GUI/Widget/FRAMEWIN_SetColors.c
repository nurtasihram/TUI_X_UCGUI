
#include "FRAMEWIN_Private.h"

void FRAMEWIN_SetBarColor(FRAMEWIN_Handle hObj, unsigned Index, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aBarColor)) {
      pObj->Props.aBarColor[Index] = Color;
      FRAMEWIN_Invalidate(hObj);
    }

  }
}
void FRAMEWIN_SetTextColor(FRAMEWIN_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;
    int i;

    pObj = FRAMEWIN_H2P(hObj);
    for (i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++) {
      pObj->Props.aTextColor[i] = Color;
    }
    FRAMEWIN_Invalidate(hObj);

  }
}
void FRAMEWIN_SetTextColorEx(FRAMEWIN_Handle hObj, unsigned Index, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
      pObj->Props.aTextColor[Index] = Color;
      FRAMEWIN_Invalidate(hObj);
    }

  }
}
void FRAMEWIN_SetClientColor(FRAMEWIN_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    if (pObj->Props.ClientColor != Color) {
      pObj->Props.ClientColor = Color;
      FRAMEWIN_Invalidate(pObj->hClient);
    }

  }
}
 /* GUI_WINSUPPORT */
