

#include "GUI_Protected.h"

int GUI_GetBkColorIndex(void) {
  int r;

  r = LCD_GetBkColorIndex();

  return r;
}

GUI_COLOR GUI_GetBkColor(void) {
  GUI_COLOR r;

  r = LCD_GetBkColorIndex();

  return r;
}

int GUI_GetColorIndex(void) {
  int r;

  r = LCD_GetColorIndex();

  return r;
}

GUI_COLOR GUI_GetColor(void) {
  GUI_COLOR r;

  r = LCD_GetColorIndex();

  return r;
}

/*************************** End of file ****************************/
