
#include "GUI.h"
#include "GUI_Protected.h"

import TUX.Widget;

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


static void _DrawBitmap(const GUI_DRAW *pObj, int x, int y) {
	GUI_DrawBitmap((CBITMAP *)pObj->Data.pData, x + pObj->xOff, y + pObj->yOff);
}
static int _GetXSize(const GUI_DRAW *pObj) {
	return ((CBITMAP *)pObj->Data.pData)->XSize;
}
static int _GetYSize(const GUI_DRAW *pObj) {
	return ((CBITMAP *)pObj->Data.pData)->YSize;
}
static const GUI_DRAW_CONSTS _ConstObjData = {
	_DrawBitmap,
	_GetXSize,
	_GetYSize
};
GUI_DRAW *GUI_DRAW_BITMAP_Create(PCBITMAP pBitmap, int x, int y) {
	auto pObj = (GUI_DRAW *)GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
	if (pObj) {
		pObj->pConsts = &_ConstObjData;
		pObj->Data.pData = (const void *)pBitmap;
		pObj->xOff = x;
		pObj->yOff = y;
	}
	return pObj;
}


static void _DrawSELF(const GUI_DRAW *pObj, int x, int y) {
	GUI_USE_PARA(x);
	GUI_USE_PARA(y);
	(*pObj->Data.pfDraw)();
}
static int _GetXSizeSELF(const GUI_DRAW *pObj) {
	GUI_USE_PARA(pObj);
	return 0;
}
static int _GetYSizeSELF(const GUI_DRAW *pObj) {
	GUI_USE_PARA(pObj);
	return 0;
}
static const GUI_DRAW_CONSTS _ConstObjDataSELF = {
	_DrawSELF,
	_GetXSizeSELF,
	_GetYSizeSELF
};
GUI_DRAW *GUI_DRAW_SELF_Create(GUI_DRAW_SELF_CB *pfDraw, int x, int y) {
	auto pObj = (GUI_DRAW *)GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
	if (pObj) {
		pObj->pConsts = &_ConstObjDataSELF;
		pObj->Data.pfDraw = pfDraw;
		pObj->xOff = x;
		pObj->yOff = y;
	}
	return pObj;
}
