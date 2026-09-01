#pragma once

#include <stdint.h>
#include "GUI_ConfDefaults.h" /* Used for  */

import TUX.Types;

typedef void tLCDDEV_FillRect(RECT r);
typedef RGBC tLCDDEV_GetPixel(int x, int y);
typedef void tLCDDEV_SetPixel(int x, int y, RGBC Color);
typedef RECT tLCDDEV_GetRect();

typedef void tLCDDEV_DrawBitmap(int x0, int y0, int xsize, int ysize,
								int BitsPerPixel, int BytesPerLine,
								const uint8_t *pData, int Diff,
								PCLOGPALETTE pTrans);

 struct tLCDDEV_APIList {
	tLCDDEV_DrawBitmap *pfDrawBitmap;
	tLCDDEV_FillRect *pfFillRect;
	tLCDDEV_GetPixel *pfGetPixel;
	tLCDDEV_GetRect *pfGetRect;
	tLCDDEV_SetPixel *pfSetPixel;
#if GUI_SUPPORT_MEMDEV
	const tLCDDEV_APIList *pMemDevAPI;
	unsigned BitsPerPixel;
#endif
};

extern const tLCDDEV_APIList GUI_MEMDEV__APIList24;

void LCD_SetClipRectEx(const RECT *pRect);
void LCD_SetClipRectMax(void);

void LCD_SetPixel(int x, int y, int Color);
RGBC LCD_GetPixel(int x, int y);

void LCD_FillRect(RECT r);

void LCD_X_Init(void);
