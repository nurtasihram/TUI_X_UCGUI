

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"

char GUI_IsInFont(const GUI_FONT GUI_UNI_PTR *pFont, U16 c) {
  if (pFont == NULL) {
    pFont = GUI_Context.pAFont;
  }
  return pFont->pfIsInFont(pFont, c);
}

/*************************** End of file ****************************/
