

#include "GUI_Protected.h"

int GUI_GetYSizeOfFont(const GUI_FONT GUI_UNI_PTR * pFont) {
  return pFont->YSize * pFont->YMag;
}

int GUI_GetYDistOfFont(const GUI_FONT GUI_UNI_PTR * pFont) {
  return pFont->YDist * pFont->YMag;
}


