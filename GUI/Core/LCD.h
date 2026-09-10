#pragma once

#include <stdint.h>
#include "GUIConf.h"

import TUX.Types;
import TUX.LCD;

extern LCDDEV *pLCD_API;

void LCD_DrawBitmap(BITVIEW b);
void LCD_SetPixel(int x, int y, RGBC Color);
void LCD_FillRect(RECT r);
