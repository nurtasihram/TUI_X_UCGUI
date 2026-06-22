

#include "LCD_Private.h"
#include "GUI_Private.h"

void LCD_UpdateColorIndices(void) {
  LCD_SetColorIndex(GUI_Context.Color);
  LCD_SetBkColorIndex(GUI_Context.BkColor);
}


