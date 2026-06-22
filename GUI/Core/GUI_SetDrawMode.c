

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Protected.h"
#include "GUIDebug.h"

GUI_DRAWMODE GUI_SetDrawMode(GUI_DRAWMODE dm) {
  GUI_DRAWMODE OldMode;
   {
  OldMode = LCD_SetDrawMode(dm);
  }
  return OldMode;
}


