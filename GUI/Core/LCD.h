#pragma once

#include "GUI_ConfDefaults.h" /* Used for GUI_UNI_PTR */

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#define I8    signed char
#define U8  unsigned char     /* unsigned 8  bits. */
#define I16   signed short    /*   signed 16 bits. */
#define U16 unsigned short    /* unsigned 16 bits. */
#define I32   signed long   /*   signed 32 bits. */
#define U32 unsigned long   /* unsigned 32 bits. */
#define I16P I16              /*   signed 16 bits OR MORE ! */
#define U16P U16              /* unsigned 16 bits OR MORE ! */

#define LCD_DRAWMODE_NORMAL (0)
#define LCD_DRAWMODE_XOR    (1<<0)
#define LCD_DRAWMODE_TRANS  (1<<1)
#define LCD_DRAWMODE_REV    (1<<2)

typedef int LCD_DRAWMODE;
typedef U32 LCD_COLOR;

typedef struct { I16P x,y; } GUI_POINT;
typedef struct { I16 x0,y0,x1,y1; } LCD_RECT;

typedef struct {
  int              NumEntries; 
  char             HasTrans;         
  const LCD_COLOR GUI_UNI_PTR * pPalEntries; 
} LCD_LOGPALETTE; 

/* This is used for the simulation only ! */
typedef struct {
  int x,y;
  unsigned char KeyStat;
} LCD_tMouseState;

/*********************************************************************
*
*     Index2Color

  This function needs to be int the public part of the software
  since it is needed by the simulation. Most other driver
  functions are hidden in the private header file.
*/

typedef LCD_COLOR      tLCDDEV_Index2Color  (int Index);
typedef unsigned int   tLCDDEV_Color2Index  (LCD_COLOR Color);
typedef unsigned int   tLCDDEV_GetIndexMask (void);

LCD_COLOR      LCD_L0_Index2Color  (int Index);
unsigned int   LCD_L0_Color2Index  (LCD_COLOR Color);
unsigned int   LCD_L0_GetIndexMask  (void);

/*********************************************************************
*
*     Color conversion API tables
*/

typedef struct {
  tLCDDEV_Color2Index*  pfColor2Index;
  tLCDDEV_Index2Color*  pfIndex2Color;
  tLCDDEV_GetIndexMask* pfGetIndexMask;
} LCD_API_COLOR_CONV;

extern const LCD_API_COLOR_CONV LCD_API_ColorConv_1;
extern const LCD_API_COLOR_CONV LCD_API_ColorConv_2;
extern const LCD_API_COLOR_CONV LCD_API_ColorConv_4;
extern const LCD_API_COLOR_CONV LCD_API_ColorConv_8666;

#define GUI_COLOR_CONV_1    &LCD_API_ColorConv_1
#define GUI_COLOR_CONV_2    &LCD_API_ColorConv_2
#define GUI_COLOR_CONV_4    &LCD_API_ColorConv_4
#define GUI_COLOR_CONV_8666 &LCD_API_ColorConv_8666

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
typedef void         tLCDDEV_DrawHLine    (int x0, int y0,  int x1);
typedef void         tLCDDEV_DrawVLine    (int x , int y0,  int y1);
typedef void         tLCDDEV_FillRect     (int x0, int y0, int x1, int y1);
typedef unsigned int tLCDDEV_GetPixelIndex(int x, int y);
typedef void         tLCDDEV_SetPixelIndex(int x, int y, int ColorIndex);
typedef void         tLCDDEV_XorPixel     (int x, int y);
typedef void         tLCDDEV_FillPolygon  (const GUI_POINT* pPoints, int NumPoints, int x0, int y0);
typedef void         tLCDDEV_FillPolygonAA(const GUI_POINT* pPoints, int NumPoints, int x0, int y0);
typedef void         tLCDDEV_GetRect      (LCD_RECT*pRect);

/*********************************************************************
*
*     Memory device API tables
*/
#if GUI_COMPILER_SUPPORTS_FP
  typedef struct tLCDDEV_APIList_struct tLCDDEV_APIList;
#endif

typedef void tLCDDEV_DrawBitmap   (int x0, int y0, int xsize, int ysize,
                       int BitsPerPixel, int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const void* pTrans);   /* Really LCD_PIXELINDEX, but is void to avoid compiler warnings*/

struct tLCDDEV_APIList_struct {
  tLCDDEV_Color2Index*        pfColor2Index;
  tLCDDEV_Index2Color*        pfIndex2Color;
  tLCDDEV_GetIndexMask*       pfGetIndexMask;
  tLCDDEV_DrawBitmap*         pfDrawBitmap;
  tLCDDEV_DrawHLine*          pfDrawHLine;
  tLCDDEV_DrawVLine*          pfDrawVLine;
  tLCDDEV_FillRect*           pfFillRect;
  tLCDDEV_GetPixelIndex*      pfGetPixelIndex;
  tLCDDEV_GetRect*            pfGetRect;
  tLCDDEV_SetPixelIndex*      pfSetPixelIndex;
  tLCDDEV_XorPixel*           pfXorPixel;
  #if GUI_SUPPORT_MEMDEV
    tLCDDEV_FillPolygon*      pfFillPolygon;
    tLCDDEV_FillPolygonAA*    pfFillPolygonAA;
    const tLCDDEV_APIList*    pMemDevAPI;
    unsigned                  BitsPerPixel;
  #endif
};
#if GUI_COMPILER_SUPPORTS_FP
  extern const struct tLCDDEV_APIList_struct GUI_MEMDEV__APIList1;
  extern const struct tLCDDEV_APIList_struct GUI_MEMDEV__APIList8;
  extern const struct tLCDDEV_APIList_struct GUI_MEMDEV__APIList16;

  #define GUI_MEMDEV_APILIST_1  &GUI_MEMDEV__APIList1
  #define GUI_MEMDEV_APILIST_8  &GUI_MEMDEV__APIList8
  #define GUI_MEMDEV_APILIST_16 &GUI_MEMDEV__APIList16
#endif

int LCD_GetXSize(void);
int LCD_GetYSize(void);

U32 LCD_GetNumColors(void);

int LCD_GetBitsPerPixel(void);
int LCD_GetFixedPalette(void);

#define LCD_GET_XSIZE()        LCD_GetXSize()
#define LCD_GET_YSIZE()        LCD_GetYSize()
#define LCD_GET_NUMCOLORS()    LCD_GetNumColors()
#define LCD_GET_BITSPERPIXEL() LCD_GetBitsPerPixel()


/*********************************************************************
*
*      LCD_CLIP function table
*
**********************************************************************
*/
typedef void         tLCD_HL_DrawHLine    (int x0, int y0,  int x1);
typedef void         tLCD_HL_DrawPixel    (int x0, int y0);

typedef struct {
  tLCD_HL_DrawHLine*          pfDrawHLine;
  tLCD_HL_DrawPixel*          pfDrawPixel;
} tLCD_HL_APIList;

void LCD_DrawHLine(int x0, int y0,  int x1);
void LCD_DrawPixel(int x0, int y0);
void LCD_DrawVLine  (int x, int y0,  int y1);



/*********************************************************************
*
*              Declarations for LCD_
*
**********************************************************************
*/

void LCD_SetClipRectEx(const LCD_RECT* pRect);
void LCD_SetClipRectMax(void);

/* Get device capabilities (0 if not supported) */
I32  LCD_GetDevCap(int Index);

/* Initialize LCD using config-paramters */
int LCD_Init(void);

void LCD_SetBkColor   (LCD_COLOR Color); /* Set background color */
void LCD_SetColor     (LCD_COLOR Color); /* Set foreground color */
void LCD_SetPixelIndex(int x, int y, int ColorIndex);

LCD_DRAWMODE LCD_SetDrawMode  (LCD_DRAWMODE dm);
void LCD_SetColorIndex(int Index);
void LCD_SetBkColorIndex(int Index);
void LCD_FillRect(int x0, int y0, int x1, int y1);
typedef void tLCD_SetPixelAA(int x, int y, U8 Intens);

LCD_COLOR    LCD_MixColors256(LCD_COLOR Color, LCD_COLOR BkColor, unsigned Intens);
LCD_COLOR    LCD_GetPixelColor(int x, int y);     /* Get RGB color of pixel */
unsigned int LCD_GetPixelIndex(int x, int y);
int          LCD_GetBkColorIndex (void);
int          LCD_GetColorIndex (void);

/*      *************************************************************
        *                                                           *
        *      LCD  imports                                         *
        *                  (for routines in LCDColor)               *
        *                                                           *
        *************************************************************
*/
  

int              LCD_Color2Index     (LCD_COLOR Color);
LCD_COLOR        LCD_Index2Color     (int Index);

/*********************************************************************
*
*       LCD_X_...
*
**********************************************************************
*/

void LCD_X_Init(void);

#if defined(__cplusplus)
  }
#endif
