#pragma once

#include <stdint.h>

#include "GUI_ConfDefaults.h" /* Used for  */

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#define DRAWMODE_NORMAL (0)
#define DRAWMODE_TRANS  (1<<1)
#define DRAWMODE_REV    (1<<2)

typedef int GUI_DRAWMODE;
typedef uint32_t RGB_COLOR;

typedef struct { int16_t x, y; } GUI_POINT;
typedef struct { int16_t x0, y0, x1, y1; } GUI_RECT;

typedef struct {
  int  NumEntries;
  char HasTrans;
  const RGB_COLOR  * pPalEntries;
} GUI_LOGPALETTE;

/* This is used for the simulation only ! */
typedef struct {
  int16_t x, y;
  uint8_t KeyStat;
} LCD_tMouseState;

/*********************************************************************
*
*      LCDDEV function table
*
**********************************************************************

  Below the routines which need to in an LCDDEV routine table are
  defined. All of these routines have to be in the low-level driver
  (LCD_L0) or in the memory device which can be used to replace the
  driver.
  The one exception to this is the SetClipRect routine, which would
  be identical for all drivers and is therefor contained in the
  level above (LCD).
*/
typedef void      tLCDDEV_DrawHLine(int x0, int y0,  int x1);
typedef void      tLCDDEV_DrawVLine(int x , int y0,  int y1);
typedef void      tLCDDEV_FillRect(int x0, int y0, int x1, int y1);
typedef RGB_COLOR tLCDDEV_GetPixel(int x, int y);
typedef void      tLCDDEV_SetPixel(int x, int y, RGB_COLOR Color);
typedef void      tLCDDEV_GetRect(GUI_RECT *pRect);

typedef void tLCDDEV_DrawBitmap(int x0, int y0, int xsize, int ysize,
                                int BitsPerPixel, int BytesPerLine,
                                const uint8_t  * pData, int Diff,
                                const void* pTrans);

typedef struct tLCDDEV_APIList_struct {
  tLCDDEV_DrawBitmap *pfDrawBitmap;
  tLCDDEV_DrawHLine  *pfDrawHLine;
  tLCDDEV_DrawVLine  *pfDrawVLine;
  tLCDDEV_FillRect   *pfFillRect;
  tLCDDEV_GetPixel   *pfGetPixel;
  tLCDDEV_GetRect    *pfGetRect;
  tLCDDEV_SetPixel   *pfSetPixel;
#if GUI_SUPPORT_MEMDEV
    const struct tLCDDEV_APIList_struct *pMemDevAPI;
    unsigned BitsPerPixel;
#endif
} tLCDDEV_APIList;

extern const tLCDDEV_APIList GUI_MEMDEV__APIList16;

void LCD_DrawHLine(int x0, int y0,  int x1);
void LCD_DrawPixel(int x0, int y0);
void LCD_DrawVLine(int x, int y0,  int y1);

void LCD_SetClipRectEx(const GUI_RECT* pRect);
void LCD_SetClipRectMax(void);

/* Initialize LCD using config-paramters */
int LCD_Init(void);

void LCD_SetPixel(int x, int y, int Color);
RGB_COLOR LCD_GetPixel(int x, int y);

GUI_DRAWMODE LCD_SetDrawMode(GUI_DRAWMODE dm);
void LCD_FillRect(int x0, int y0, int x1, int y1);

RGB_COLOR LCD_MixColors256(RGB_COLOR Color, RGB_COLOR BkColor, unsigned Intens);

void LCD_X_Init(void);

#if defined(__cplusplus)
  }
#endif
