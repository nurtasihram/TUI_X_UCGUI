#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"
#include "GUIDebug.h"
#include "LCD_Private.h"      /* Required for configuration, APIList */

const tLCDDEV_APIList LCD_L0_APIList = {
  (tLCDDEV_DrawBitmap*)LCD_L0_DrawBitmap,
  LCD_L0_DrawHLine,
  LCD_L0_DrawVLine,
  LCD_L0_FillRect,
  LCD_L0_GetPixelIndex,
  LCD_L0_GetRect,
  LCD_L0_SetPixelIndex,
  LCD_L0_XorPixel,
#if GUI_SUPPORT_MEMDEV
  NULL,                       /* pfFillPolygon */
  &GUI_MEMDEV__APIList16
#endif
};

const tLCDDEV_APIList* /*const*/ LCD_aAPI[] = {
  &LCD_L0_APIList
};
