/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              �C/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : LCD_Protected.h
Purpose     : Internals of the LCD level
----------------------------------------------------------------------
*/

#ifndef LCD_INTERN_H
#define LCD_INTERN_H

#include "LCD.h"

/*********************************************************************
*
*      Data types
*
**********************************************************************
*/

typedef struct {
  int              NumEntries; 
  const LCD_COLOR* pPalEntries; 
} LCD_PHYSPALETTE; 

/*********************************************************************
*
*      Data
*
**********************************************************************
*/
extern GUI_CONST_STORAGE U8 LCD_aMirror[256];

void LCD_SelectLCD(void);

/*********************************************************************
*
*              Declarations for LCD_L0_
*
**********************************************************************

  Prefixes are as follows:
  LCD_L0_xxx       ... Driver for first  LCD
  LCD_L0_1_xxx     ... Driver for second LCD/layer
  LCD_L0_2_xxx     ... Driver for third  LCD/layer
  LCD_L0_MAG_xxx   ... Driver for first  LCD, magnification level (calling LCD_L0_xxx after magnification)
  LCD_L0_DELTA_xxx ... Driver for first  LCD, delta mode

*/

unsigned int LCD_L0_Color2Index       (LCD_COLOR Color);
unsigned int LCD_L0_1_Color2Index     (LCD_COLOR Color);
unsigned int LCD_L0_2_Color2Index     (LCD_COLOR Color);
unsigned int LCD_L0_3_Color2Index     (LCD_COLOR Color);
unsigned int LCD_L0_4_Color2Index     (LCD_COLOR Color);
void         LCD_L0_DrawHLine         (int x0, int y0,  int x1);
void         LCD_L0_1_DrawHLine       (int x0, int y0,  int x1);
void         LCD_L0_2_DrawHLine       (int x0, int y0,  int x1);
void         LCD_L0_3_DrawHLine       (int x0, int y0,  int x1);
void         LCD_L0_4_DrawHLine       (int x0, int y0,  int x1);
void         LCD_L0_MAG_DrawHLine     (int x0, int y0,  int x1);
void         LCD_L0_DELTA_DrawHLine     (int x0, int y0,  int x1);
void         LCD_L0_DrawVLine         (int x,  int y0,  int y1);
void         LCD_L0_1_DrawVLine       (int x,  int y0,  int y1);
void         LCD_L0_2_DrawVLine       (int x,  int y0,  int y1);
void         LCD_L0_3_DrawVLine       (int x,  int y0,  int y1);
void         LCD_L0_4_DrawVLine       (int x,  int y0,  int y1);
void         LCD_L0_MAG_DrawVLine     (int x,  int y0,  int y1);
void         LCD_L0_DELTA_DrawVLine     (int x,  int y0,  int y1);
void         LCD_L0_FillRect          (int x0, int y0, int x1, int y1);
void         LCD_L0_1_FillRect        (int x0, int y0, int x1, int y1);
void         LCD_L0_2_FillRect        (int x0, int y0, int x1, int y1);
void         LCD_L0_3_FillRect        (int x0, int y0, int x1, int y1);
void         LCD_L0_4_FillRect        (int x0, int y0, int x1, int y1);
void         LCD_L0_MAG_FillRect      (int x0, int y0, int x1, int y1);
void         LCD_L0_DELTA_FillRect      (int x0, int y0, int x1, int y1);
unsigned int LCD_L0_GetPixelIndex     (int x,  int y);
unsigned int LCD_L0_1_GetPixelIndex   (int x,  int y);
unsigned int LCD_L0_2_GetPixelIndex   (int x,  int y);
unsigned int LCD_L0_3_GetPixelIndex   (int x,  int y);
unsigned int LCD_L0_4_GetPixelIndex   (int x,  int y);
unsigned int LCD_L0_MAG_GetPixelIndex (int x,  int y);
unsigned int LCD_L0_DELTA_GetPixelIndex (int x,  int y);
void         LCD_L0_GetRect           (LCD_RECT * pRect);
void         LCD_L0_1_GetRect         (LCD_RECT * pRect);
void         LCD_L0_2_GetRect         (LCD_RECT * pRect);
void         LCD_L0_3_GetRect         (LCD_RECT * pRect);
void         LCD_L0_4_GetRect         (LCD_RECT * pRect);
void         LCD_L0_SetPixelIndex     (int x,  int y, int ColorIndex);
void         LCD_L0_1_SetPixelIndex   (int x,  int y, int ColorIndex);
void         LCD_L0_2_SetPixelIndex   (int x,  int y, int ColorIndex);
void         LCD_L0_3_SetPixelIndex   (int x,  int y, int ColorIndex);
void         LCD_L0_4_SetPixelIndex   (int x,  int y, int ColorIndex);
void         LCD_L0_MAG_SetPixelIndex (int x,  int y, int ColorIndex);
void         LCD_L0_DELTA_SetPixelIndex (int x,  int y, int ColorIndex);
void         LCD_L0_XorPixel          (int x,  int y);
void         LCD_L0_1_XorPixel        (int x,  int y);
void         LCD_L0_2_XorPixel        (int x,  int y);
void         LCD_L0_3_XorPixel        (int x,  int y);
void         LCD_L0_4_XorPixel        (int x,  int y);
void         LCD_L0_MAG_XorPixel      (int x,  int y);
void         LCD_L0_DELTA_XorPixel      (int x,  int y);
int          LCD_L0_Init              (void);
int          LCD_L0_1_Init            (void);
int          LCD_L0_2_Init            (void);
int          LCD_L0_3_Init            (void);
int          LCD_L0_4_Init            (void);
int          LCD_L0_MAG_Init          (void);
int          LCD_L0_DELTA_Init          (void);
void         LCD_L0_On                (void);
void         LCD_L0_1_On              (void);
void         LCD_L0_2_On              (void);
void         LCD_L0_3_On              (void);
void         LCD_L0_4_On              (void);
void         LCD_L0_Off               (void);
void         LCD_L0_1_Off             (void);
void         LCD_L0_2_Off             (void);
void         LCD_L0_3_Off             (void);
void         LCD_L0_4_Off             (void);
void         LCD_L0_SetLUTEntry       (U8 Pos, LCD_COLOR color);
void         LCD_L0_1_SetLUTEntry     (U8 Pos, LCD_COLOR color);
void         LCD_L0_2_SetLUTEntry     (U8 Pos, LCD_COLOR color);
void         LCD_L0_3_SetLUTEntry     (U8 Pos, LCD_COLOR color);
void         LCD_L0_4_SetLUTEntry     (U8 Pos, LCD_COLOR color);

void LCD_L0_ReInit       (void); /* Reinitialize LCD using config-paramters */
void LCD_L0_Refresh      (void); /* Refresh entire LCD */
                                 /* Refresh LCD partially. Could be one line, depends on what
                                    the LCD controller can handle efficiently */
void LCD_L0_SetOrg       (int x, int y);


#endif /* LCD_INTERN_H */



/*************************** End of file ****************************/

