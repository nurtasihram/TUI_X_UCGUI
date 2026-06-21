

#include "GUI_Protected.h"
#include "GUIDebug.h"

const GUI_FONT GUI_UNI_PTR* GUI_SetFont(const GUI_FONT GUI_UNI_PTR * pNewFont) {
  const GUI_FONT GUI_UNI_PTR* pOldFont = GUI_Context.pAFont;

  if (pNewFont)
    GUI_Context.pAFont = pNewFont;

  return pOldFont;
}

/*************************** End of file ****************************/
