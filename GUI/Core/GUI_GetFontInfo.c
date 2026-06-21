

#include "GUI_Protected.h"
#include <stddef.h>

#if GUI_COMPILER_SUPPORTS_FP

void GUI_GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO * pFontInfo) {

  if (pFont == NULL) {
    pFont = GUI_Context.pAFont;
  }
  pFontInfo->Baseline = pFont->Baseline;
  pFontInfo->CHeight  = pFont->CHeight;
  pFontInfo->LHeight  = pFont->LHeight;
  pFont->pfGetFontInfo(pFont, pFontInfo);

}

#endif

/*************************** End of file ****************************/
