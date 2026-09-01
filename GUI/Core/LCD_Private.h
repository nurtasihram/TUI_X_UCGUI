#pragma once

#include "LCD_Protected.h"
#include "GUI.h"

extern const struct tLCDDEV_APIList_struct *LCD_aAPI[1];

#define LCD_BKCOLORINDEX GUI.aColor[0]
#define LCD_COLORINDEX   GUI.aColor[1]
#define LCD_ACOLORINDEX  GUI.aColor

void LCD_DrawBitmap(int x0, int y0,
                    int xsize, int ysize,
                    int BitsPerPixel, int BytesPerLine,
                    const void *pPixel,
                    PCLOGPALETTE pTrans);
