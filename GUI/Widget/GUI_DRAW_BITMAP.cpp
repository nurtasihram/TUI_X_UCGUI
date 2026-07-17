
#include "GUI.h"
#include "GUI_Protected.h"
#include "WIDGET.h"

static void _DrawBitmap(const GUI_DRAW* pObj, int x, int y) {
  GUI_DrawBitmap((const GUI_BITMAP *)pObj->Data.pData, x + pObj->xOff, y + pObj->yOff);
}
static int _GetXSize(const GUI_DRAW* pObj) {
  return ((const GUI_BITMAP *)pObj->Data.pData)->XSize;
}
static int _GetYSize(const GUI_DRAW* pObj) {
  return ((const GUI_BITMAP *)pObj->Data.pData)->YSize;
}
static const GUI_DRAW_CONSTS _ConstObjData = {
  _DrawBitmap,
  _GetXSize,
  _GetYSize
};
WM_HMEM GUI_DRAW_BITMAP_Create(const GUI_BITMAP* pBitmap, int x, int y) {
  GUI_DRAW* pObj;
  WM_HMEM hMem;
  hMem = GUI_ALLOC_AllocZero(sizeof(GUI_DRAW));
  if (hMem) {

    pObj = (GUI_DRAW*)(hMem);
    pObj->pConsts    = &_ConstObjData;
    pObj->Data.pData = (const void*)pBitmap;
    pObj->xOff       = x;
    pObj->yOff       = y;

  }
  return hMem;
}
