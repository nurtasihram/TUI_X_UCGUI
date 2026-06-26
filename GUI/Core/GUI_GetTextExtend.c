

#include "GUI_Protected.h"

void GUI_GetTextExtend(GUI_RECT* pRect, const char  * s, int MaxNumChars) {
  int xMax      = 0;
  int NumLines  = 0;
  int LineSizeX = 0;
  uint16_t Char;
  pRect->x0 = GUI_Context.DispPosX;
  pRect->y0 = GUI_Context.DispPosY;
  while (MaxNumChars--) {
    Char = GUI_UC__GetCharCodeInc(&s);
    if ((Char == '\n') || (Char == 0)) {
      if (LineSizeX > xMax) {
        xMax = LineSizeX;
      }
      LineSizeX = 0;
      NumLines++;
      if (!Char) {
        break;
      }
    } else {
      LineSizeX += GUI_GetCharDistX(Char);
    }
  }
  if (LineSizeX > xMax) {
    xMax = LineSizeX;
  }
  if (!NumLines) {
    NumLines = 1;
  }
  pRect->x1 = pRect->x0 + xMax - 1;
  pRect->y1 = pRect->y0 + GUI__GetFontSizeY() * NumLines - 1;
}


