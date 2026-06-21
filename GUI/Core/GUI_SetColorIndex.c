

#include "GUI_Protected.h"

void GUI_SetBkColorIndex(int Index) {
   {
    GUI_Context.BkColor = GUI_INVALID_COLOR;
    LCD_SetBkColorIndex(Index);
  }
}

void GUI_SetColorIndex(int Index) {
   {
    GUI_Context.Color = GUI_INVALID_COLOR;
    LCD_SetColorIndex(Index);
  }
}

/*************************** End of file ****************************/
