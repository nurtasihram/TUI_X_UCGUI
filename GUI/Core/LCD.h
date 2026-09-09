#pragma once

#include <stdint.h>
#include "GUIConf.h"

import TUX.Types;

struct tLCDDEV_APIList {
	uint8_t BitsPerPixel;
	tLCDDEV_APIList *pMemDevAPI = nullptr;
	tLCDDEV_APIList(tLCDDEV_APIList *pMemDevAPI, uint8_t BitsPerPixel)
		: BitsPerPixel(BitsPerPixel), pMemDevAPI(pMemDevAPI) {}
	virtual RECT GetRect() = 0;
	virtual RGBC GetPixel(int x, int y) = 0;
	virtual void SetPixel(int x, int y, RGBC Color) = 0;
	virtual void DrawBitmap(BITVIEW b) = 0;
	virtual void FillRect(RECT r) = 0;
};

extern tLCDDEV_APIList *pMEMDEV__APIList24;
extern tLCDDEV_APIList *pLCD_API;

void LCD_DrawBitmap(BITVIEW b);

void LCD_SetPixel(int x, int y, int Color);
void LCD_FillRect(RECT r);

void LCD_SelectLCD(void);

tLCDDEV_APIList *LCD_L0_Init(void);
