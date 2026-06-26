#pragma once

#include "LCD_Protected.h"
#include "GUI.h"

extern const struct tLCDDEV_APIList_struct * /* const */ LCD_aAPI[1];

#define LCD_BKCOLORINDEX GUI_Context.aColorIndex[0]
#define LCD_COLORINDEX   GUI_Context.aColorIndex[1]
#define LCD_ACOLORINDEX  GUI_Context.aColorIndex

void LCD_DrawBitmap   (int x0, int y0,
                       int xsize, int ysize,
                       int BitsPerPixel, int BytesPerLine,
                       const uint8_t  * pPixel,
                       const RGB_COLOR* pTrans);
