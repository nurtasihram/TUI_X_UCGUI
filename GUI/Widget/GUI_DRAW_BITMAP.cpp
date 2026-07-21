
#include "GUI.h"
#include "GUI_Protected.h"
#include "WIDGET.h"

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
