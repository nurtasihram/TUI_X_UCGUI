

#include "GUI_Protected.h"
#include "GUIDebug.h"

const GUI_FONT * GUI_SetFont(const GUI_FONT  * pNewFont) {
  const GUI_FONT * pOldFont = GUI_Context.pAFont;

  if (pNewFont)
    GUI_Context.pAFont = pNewFont;

  return pOldFont;
}


