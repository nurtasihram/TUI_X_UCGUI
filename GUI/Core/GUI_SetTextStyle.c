

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include "GUI_Protected.h"

static void GUI__cbDrawTextStyle(uint16_t Char) {
  int x0, x1;
  x1 = GUI_Context.DispPosX - 1;
  x0 = x1 - GUI_Context.pAFont->pfGetCharDistX(Char) + 1;
  /* Handle Underline */
  if (GUI_Context.TextStyle & GUI_TS_UNDERLINE) {
    int yOff = GUI_Context.pAFont->Baseline;
    if (yOff >= GUI_Context.pAFont->YSize) {
      yOff = GUI_Context.pAFont->YSize - 1;
    }
    LCD_DrawHLine(x0, GUI_Context.DispPosY + yOff, x1);
  }
  /* Handle strike thru */
  if (GUI_Context.TextStyle & GUI_TS_STRIKETHRU) {
    int yOff = GUI_Context.pAFont->Baseline - ((GUI_Context.pAFont->CHeight + 1) / 2);
    LCD_DrawHLine(x0, GUI_Context.DispPosY + yOff, x1);
  }
  /* Handle over line */
  if (GUI_Context.TextStyle & GUI_TS_OVERLINE) {
    int yOff = GUI_Context.pAFont->Baseline - GUI_Context.pAFont->CHeight - 1;
    if (yOff < 0) {
      yOff = 0;
    }
    LCD_DrawHLine(x0, GUI_Context.DispPosY + yOff, x1);
  }
}

char GUI_SetTextStyle(char Style) {
  char OldStyle;

  OldStyle = GUI_Context.TextStyle;
  GUI_pfDispCharStyle = GUI__cbDrawTextStyle;    /* Init function pointer (function in this module) */
  GUI_Context.TextStyle = Style;

  return OldStyle;
}


