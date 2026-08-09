#include "WM_Intern.h"
#include "GUIDebug.h"
void WM_EnableMemdev(WObj * pWin) {
	GUI_USE_PARA(pWin);
#if GUI_SUPPORT_MEMDEV
	if (pWin) {
		pWin->Status |= (WC_MEMDEV);
	}
#else
	GUI_DEBUG_WARN("WM_EnableMemdev: No effect because disabled in GUIConf.h (GUI_SUPPORT_MEMDEV == 0)");
#endif
}

void WM_DisableMemdev(WObj * pWin) {
	GUI_USE_PARA(pWin);
#if GUI_SUPPORT_MEMDEV
	if (pWin) {
		pWin->Status &= ~(WC_MEMDEV | WC_MEMDEV_ON_REDRAW);
	}
#else
	GUI_DEBUG_WARN("WM_EnableMemdev: No effect because disabled in GUIConf.h (GUI_SUPPORT_MEMDEV == 0)");
#endif
}
