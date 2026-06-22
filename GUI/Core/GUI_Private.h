#pragma once

#include "GUI_Protected.h"
#include "LCD_Private.h"

#if GUI_SUPPORT_DEVICES
#define LCDDEV_L0_DrawBitmap          GUI_Context.pDeviceAPI->pfDrawBitmap
#define LCDDEV_L0_DrawHLine           GUI_Context.pDeviceAPI->pfDrawHLine
#define LCDDEV_L0_DrawVLine           GUI_Context.pDeviceAPI->pfDrawVLine
#define LCDDEV_L0_DrawPixel           GUI_Context.pDeviceAPI->pfDrawPixel
#define LCDDEV_L0_FillRect            GUI_Context.pDeviceAPI->pfFillRect
#define LCDDEV_L0_GetPixel            GUI_Context.pDeviceAPI->pfGetPixel
#define LCDDEV_L0_GetRect             GUI_Context.pDeviceAPI->pfGetRect
#define LCDDEV_L0_GetPixelIndex       GUI_Context.pDeviceAPI->pfGetPixelIndex
#define LCDDEV_L0_SetPixelIndex       GUI_Context.pDeviceAPI->pfSetPixelIndex
#define LCDDEV_L0_XorPixel            GUI_Context.pDeviceAPI->pfXorPixel
#else
#define LCDDEV_L0_DrawBitmap          LCD_L0_DrawBitmap
#define LCDDEV_L0_DrawHLine           LCD_L0_DrawHLine
#define LCDDEV_L0_DrawVLine           LCD_L0_DrawVLine
#define LCDDEV_L0_DrawPixel           LCD_L0_DrawPixel
#define LCDDEV_L0_FillRect            LCD_L0_FillRect
#define LCDDEV_L0_GetPixel            LCD_L0_GetPixel
#define LCDDEV_L0_GetRect             LCD_L0_GetRect
#define LCDDEV_L0_GetPixelIndex       LCD_L0_GetPixelIndex
#define LCDDEV_L0_SetPixelIndex       LCD_L0_SetPixelIndex
#define LCDDEV_L0_XorPixel            LCD_L0_XorPixel
#endif

void LCD_L0_DrawBitmap    (int x0, int y0, int xsize, int ysize,
                           int BitsPerPixel, int BytesPerLine,
                           const uint8_t GUI_UNI_PTR * pData, int Diff, const RGB_COLOR* pTrans);

#define GUI_POS_AUTO -4095   /* Position value for auto-pos */

#if GUI_SUPPORT_MEMDEV
  void*  GUI_MEMDEV__XY2PTR  (int x,int y);
  void*  GUI_MEMDEV__XY2PTREx(GUI_MEMDEV* pDev, int x,int y);
  void   GUI_MEMDEV__WriteToActiveAt(GUI_MEMDEV_Handle hMem,int x, int y);
#endif
