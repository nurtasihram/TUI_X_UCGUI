

#include "LCD_Private.h"
#include "GUI_Private.h"

/*********************************************************************
*
*       LCD_SetClipRectEx
*
* Purpose:
*   This function is actually a driver function.
*   Since it is identical for all drivers with only one controller,
*   it is placed here.
*   For multi-controller systems, this routine is placed in the
*   distribution driver.
*/

void LCD_SetClipRectEx(const GUI_RECT* pRect) {
  LCD_RECT r;
  LCDDEV_L0_GetRect(&r);
  GUI__IntersectRects(&GUI_Context.ClipRect, pRect, &r);
}

/*************************** End of file ****************************/
