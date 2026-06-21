

#include "GUI_Protected.h"

void GL_SetDefault(void) {
  GUI_SetBkColor(GUI_DEFAULT_BKCOLOR);
  GUI_SetColor  (GUI_DEFAULT_COLOR);
  GUI_SetPenSize(1);
  GUI_SetTextAlign(0);
  GUI_SetTextMode(0);
  GUI_SetDrawMode(0);
  GUI_SetFont(GUI_DEFAULT_FONT);
  GUI_SetLineStyle(GUI_LS_SOLID);
}

void GUI_SetDefault(void) {

  GL_SetDefault();

}

/*************************** End of file ****************************/
