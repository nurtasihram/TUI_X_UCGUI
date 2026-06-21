#pragma once

#include "LCD_ConfDefaults.h"            /* Configuration header file */
#include "LCD_Protected.h"
#include "GUI.h"

typedef struct {
  LCD_COLOR * paColor;
  I16         NumEntries;
} LCD_LUT_INFO;

#if GUI_COMPILER_SUPPORTS_FP

extern const struct tLCDDEV_APIList_struct * /* const */ LCD_aAPI[1];

#endif

#define LCD_BKCOLORINDEX GUI_Context.aColorIndex[0]
#define LCD_COLORINDEX   GUI_Context.aColorIndex[1]
#define LCD_ACOLORINDEX  GUI_Context.aColorIndex

void LCD_DrawBitmap   (int x0, int y0,
                       int xsize, int ysize,
                       int xMul, int yMul,
                       int BitsPerPixel,
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pPixel,
                       const LCD_PIXELINDEX* pTrans);
void LCD_UpdateColorIndices(void);
