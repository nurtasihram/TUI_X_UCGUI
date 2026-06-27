#pragma once

#include "LCD.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t LCD_aMirror[256];

void LCD_SelectLCD(void);

int LCD_GetXSize(void);
int LCD_GetYSize(void);

void      LCD_L0_DrawHLine (int x0, int y0, int x1);
void      LCD_L0_DrawVLine (int x,  int y0, int y1);
void      LCD_L0_FillRect  (int x0, int y0, int x1, int y1);
RGB_COLOR LCD_L0_GetPixel  (int x,  int y);
void      LCD_L0_GetRect   (GUI_RECT * pRect);
void      LCD_L0_SetPixel  (int x,  int y, RGB_COLOR Color);
int       LCD_L0_Init      (void);

#ifdef __cplusplus
}
#endif
