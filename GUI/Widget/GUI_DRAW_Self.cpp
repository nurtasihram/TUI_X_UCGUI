
#include "GUI.h"
#include "GUI_Protected.h"
#include "WIDGET.h"

static void _Draw(const GUI_DRAW *pObj, int x, int y) {
	GUI_USE_PARA(x);
	GUI_USE_PARA(y);
	(*pObj->Data.pfDraw)();
}
static int _GetXSize(const GUI_DRAW *pObj) {
	GUI_USE_PARA(pObj);
	return 0;
}
static int _GetYSize(const GUI_DRAW *pObj) {
	GUI_USE_PARA(pObj);
	return 0;
}
static const GUI_DRAW_CONSTS _ConstObjData = {
	_Draw,
	_GetXSize,
	_GetYSize
};
GUI_DRAW *GUI_DRAW_SELF_Create(GUI_DRAW_SELF_CB *pfDraw, int x, int y) {
	auto pObj = (GUI_DRAW *)GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
	if (pObj) {
		pObj->pConsts = &_ConstObjData;
		pObj->Data.pfDraw = pfDraw;
		pObj->xOff = x;
		pObj->yOff = y;
	}
	return pObj;
}
