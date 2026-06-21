

#include "GUI_Private.h"

GUI_COLOR GUI_GetBitmapPixelColor(const GUI_BITMAP GUI_UNI_PTR * pBMP, unsigned x, unsigned y) {
  int Index = GUI_GetBitmapPixelIndex(pBMP, x, y);
  if (pBMP->pPal->HasTrans && (Index == 0)) {
    return GUI_INVALID_COLOR;
  }
  return pBMP->pPal->pPalEntries[Index];
}

/*************************** End of file ****************************/
