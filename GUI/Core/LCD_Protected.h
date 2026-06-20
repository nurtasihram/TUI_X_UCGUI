#pragma once

#include "LCD.h"

typedef struct {
  int              NumEntries; 
  const LCD_COLOR* pPalEntries; 
} LCD_PHYSPALETTE; 

extern GUI_CONST_STORAGE U8 LCD_aMirror[256];

void LCD_SelectLCD(void);

void         LCD_L0_DrawHLine         (int x0, int y0,  int x1);
void         LCD_L0_DrawVLine         (int x,  int y0,  int y1);
void         LCD_L0_FillRect          (int x0, int y0, int x1, int y1);
unsigned int LCD_L0_GetPixelIndex     (int x,  int y);
void         LCD_L0_GetRect           (LCD_RECT * pRect);
void         LCD_L0_SetPixelIndex     (int x,  int y, int ColorIndex);
void         LCD_L0_XorPixel          (int x,  int y);
int          LCD_L0_Init              (void);
