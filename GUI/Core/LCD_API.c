

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"
#include "GUIDebug.h"
#include "LCD_Private.h"      /* Required for configuration, APIList */

#if GUI_COMPILER_SUPPORTS_FP

/*********************************************************************
*
*             LCD Device
*       (if memory devices are supported)
*
**********************************************************************
*/

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
  #if LCD_BITSPERPIXEL <= 8
    &GUI_MEMDEV__APIList8
  #else
    &GUI_MEMDEV__APIList16
  #endif
#endif
};

/***********************************************************
*
*                 LCD_aAPI
*
* Purpose:
*  This table lists the available displays/layers by a single pointer.
*  It is important that these are non-constants, because some high level
*  software (such as the VNC server, but maybe also the mouse "Cursor" mdoule)
*  may need to override these pointers in order to link itself into the chain
*  of drawing routines.
*  However, the API tables may of course be constants.
*
*/

const tLCDDEV_APIList* /*const*/ LCD_aAPI[] = {
  &LCD_L0_APIList
};

#endif

/*************************** End of file ****************************/

