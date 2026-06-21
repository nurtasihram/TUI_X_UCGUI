
#include "GUI.h"
#include "GUI_Protected.h"
#include "WIDGET.h"

static void _Draw(const GUI_DRAW* pObj, int x, int y) {
  GUI_BMP_Draw((const GUI_BITMAP *)pObj->Data.pData, x + pObj->xOff, y + pObj->yOff);
}
static int _GetXSize(const GUI_DRAW* pObj) {
  return GUI_BMP_GetXSize(pObj->Data.pData);
}
static int _GetYSize(const GUI_DRAW* pObj) {
  return GUI_BMP_GetXSize(pObj->Data.pData);
}
static const GUI_DRAW_CONSTS _ConstObjData = {
  _Draw,
  _GetXSize,
  _GetYSize
};
WM_HMEM GUI_DRAW_BMP_Create(const void* pBMP, int x, int y) {
  WM_HMEM hMem;
  hMem = GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
  if (hMem) {
    GUI_DRAW* pObj;

    pObj = (GUI_DRAW*)GUI_ALLOC_h2p(hMem);
    pObj->pConsts      = &_ConstObjData;
    pObj->Data.pData   = pBMP;
    pObj->xOff         = x;
    pObj->yOff         = y;

  }
  return hMem;
}

