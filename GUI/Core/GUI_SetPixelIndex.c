

#include <stdio.h>
#include "GUI_Private.h"
#include "LCD_Private.h"

#define RETURN_IF_Y_OUT() \
  if (y < GUI_Context.ClipRect.y0) return;             \
  if (y > GUI_Context.ClipRect.y1) return;

#define RETURN_IF_X_OUT() \
  if (x < GUI_Context.ClipRect.x0) return;             \
  if (x > GUI_Context.ClipRect.x1) return;

/*********************************************************************
*
*       LCD_SetPixelIndex
*
* Purpose:
*   Writes 1 pixel into the display.
*/
void LCD_SetPixelIndex(int x, int y, int ColorIndex) {
  RETURN_IF_X_OUT();
  RETURN_IF_Y_OUT();
  LCDDEV_L0_SetPixelIndex(x, y, ColorIndex);
}

/*************************** End of file ****************************/
