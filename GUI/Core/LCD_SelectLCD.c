

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"
#if GUI_WINSUPPORT
#include "WM.h"
#endif

void LCD_SelectLCD(void) {
#if GUI_SUPPORT_DEVICES
    GUI_Context.pDeviceAPI   = LCD_aAPI[0];
    GUI_Context.hDevData = 0;
#endif
  GUI_Context.pClipRect_HL = &GUI_Context.ClipRect;
  LCD_SetClipRectMax();
}


