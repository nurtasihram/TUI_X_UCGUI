
#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"
#include "WIDGET.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

int FRAMEWIN_IsMinimized(FRAMEWIN_Handle hObj) {
  int r = 0;
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    r = (pObj->Flags & FRAMEWIN_SF_MINIMIZED) ? 1 : 0;

  }
  return r;
}
int FRAMEWIN_IsMaximized(FRAMEWIN_Handle hObj) {
  int r = 0;
  if (hObj) {
    FRAMEWIN_Obj* pObj;

    pObj = FRAMEWIN_H2P(hObj);
    r = (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) ? 1 : 0;

  }
  return r;
}
#else
  void FRAMEWIN_IsMinMax_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
