
#include "GUI.h"
#include "GUI_Protected.h"
#include "WIDGET.h"

void GUI_DRAW__Draw(GUI_DRAW *pDrawObj, int x, int y) {
	if (pDrawObj)
		pDrawObj->pConsts->pfDraw(pDrawObj, x, y);
}
int GUI_DRAW__GetXSize(GUI_DRAW *pDrawObj) {
	if (pDrawObj)
		return pDrawObj->pConsts->pfGetXSize(pDrawObj);
	return 0;
}
int GUI_DRAW__GetYSize(GUI_DRAW *pDrawObj) {
	if (pDrawObj)
		return pDrawObj->pConsts->pfGetYSize(pDrawObj);
  return 0;
}
