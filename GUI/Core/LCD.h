#pragma once

#include <stdint.h>
#include "GUIConf.h"

import TUX.Types;
import TUX.LCD;

extern LCDDEV_API *pMEMDEV__APIList24;
extern LCDDEV_API *pLCD_API;

void LCD_DrawBitmap(BITVIEW b);
void LCD_SetPixel(int x, int y, RGBC Color);
void LCD_FillRect(RECT r);
