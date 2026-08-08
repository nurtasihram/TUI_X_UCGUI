#pragma once

#include <stdint.h>
#include "GUI_ConfDefaults.h" /* Used for  */

import TUX.Types;

typedef void tLCDDEV_DrawHLine(int x0, int y0, int x1);
typedef void tLCDDEV_DrawVLine(int x, int y0, int y1);
typedef void tLCDDEV_FillRect(int x0, int y0, int x1, int y1);
typedef RGBC tLCDDEV_GetPixel(int x, int y);
typedef void tLCDDEV_SetPixel(int x, int y, RGBC Color);
typedef void tLCDDEV_GetRect(RECT *pRect);

typedef void tLCDDEV_DrawBitmap(int x0, int y0, int xsize, int ysize,
								int BitsPerPixel, int BytesPerLine,
								const uint8_t *pData, int Diff,
								const void *pTrans);

typedef struct tLCDDEV_APIList_struct {
	tLCDDEV_DrawBitmap *pfDrawBitmap;
	tLCDDEV_DrawHLine *pfDrawHLine;
	tLCDDEV_DrawVLine *pfDrawVLine;
	tLCDDEV_FillRect *pfFillRect;
	tLCDDEV_GetPixel *pfGetPixel;
	tLCDDEV_GetRect *pfGetRect;
	tLCDDEV_SetPixel *pfSetPixel;
#if GUI_SUPPORT_MEMDEV
	const struct tLCDDEV_APIList_struct *pMemDevAPI;
	unsigned BitsPerPixel;
#endif
} tLCDDEV_APIList;

extern const tLCDDEV_APIList GUI_MEMDEV__APIList24;

void LCD_DrawHLine(int x0, int y0, int x1);
void LCD_DrawPixel(int x0, int y0);
void LCD_DrawVLine(int x, int y0, int y1);

void LCD_SetClipRectEx(const RECT *pRect);
void LCD_SetClipRectMax(void);

void LCD_SetPixel(int x, int y, int Color);
RGBC LCD_GetPixel(int x, int y);

void LCD_FillRect(int x0, int y0, int x1, int y1);

void LCD_X_Init(void);
