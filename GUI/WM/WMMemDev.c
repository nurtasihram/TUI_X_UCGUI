

#include "WM_Intern.h"
#include "GUIDebug.h"

void WM_EnableMemdev (WM_HWIN hWin) {
  GUI_USE_PARA(hWin);
  #if GUI_SUPPORT_MEMDEV
    if (hWin) {
      WM_Obj * pWin;

      pWin = WM_HANDLE2PTR(hWin);
      pWin->Status |= (WM_SF_MEMDEV);

    }
  #else
    GUI_DEBUG_WARN("WM_EnableMemdev: No effect because disabled in GUIConf.h (GUI_SUPPORT_MEMDEV == 0)");
  #endif
}

void WM_DisableMemdev(WM_HWIN hWin) {
  GUI_USE_PARA(hWin);
  #if GUI_SUPPORT_MEMDEV
    if (hWin) {
      WM_Obj * pWin;

      pWin = WM_HANDLE2PTR(hWin);
      pWin->Status &= ~(WM_SF_MEMDEV | WM_SF_MEMDEV_ON_REDRAW);

    }
  #else
    GUI_DEBUG_WARN("WM_EnableMemdev: No effect because disabled in GUIConf.h (GUI_SUPPORT_MEMDEV == 0)");
  #endif
}

/*************************** End of file ****************************/
