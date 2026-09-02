#pragma once

#include <stdint.h>
#include "GUI_ConfDefaults.h"

import TUX.Types;

 struct tLCDDEV_APIList {
	 void (*pfDrawBitmap)(int x0, int y0, int xsize, int ysize,
						  int BitsPerPixel, int BytesPerLine,
						  const uint8_t *pData, int Diff,
						  PCLOGPALETTE pTrans);
	void(* pfFillRect)(RECT r);
	RGBC(* pfGetPixel)(int x, int y);
	void(* pfSetPixel)(int x, int y, RGBC Color);
	RECT(* pfGetRect)();
	uint8_t BitsPerPixel;
#if GUI_SUPPORT_MEMDEV
	const tLCDDEV_APIList *pMemDevAPI = nullptr;
#endif
};

extern const tLCDDEV_APIList GUI_MEMDEV__APIList24;
extern const tLCDDEV_APIList LCD_API;

extern const uint8_t LCD_aMirror[256];

void LCD_DrawBitmap(int x0, int y0,
					int xsize, int ysize,
					int BitsPerPixel, int BytesPerLine,
					const void *pPixel,
					PCLOGPALETTE pTrans);

void LCD_SetPixel(int x, int y, int Color);
RGBC LCD_GetPixel(int x, int y);

void LCD_FillRect(RECT r);

void LCD_SelectLCD(void);

bool LCD_L0_Init(void);
