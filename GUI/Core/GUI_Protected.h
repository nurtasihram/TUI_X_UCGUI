#pragma once

#include "GUI.h"
#include "LCD_Protected.h"
#include "GUIDebug.h"

#include "WM_GUI.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/**********************************************************************
*
*                        Defaults for config switches
*
***********************************************************************

  The config switches below do not affect the interface in GUI.h and
  are therefor not required to be in GUI.h.
*/

/* Short address area.
   For  most compilers, this is "near" or "__near"
   We do not use this except for some CPUs which we know to always have some
   near memory, because the GUI_Context ans some other data will be declared
   to be in this short address (near) memory area as it has a major effect
   on performance.
   Please define in GUIConf.h (if you want to use it)
*/
#ifndef GUI_SADDR
#define GUI_SADDR
#endif

#ifndef GUI_DEFAULT_FONT
#define GUI_DEFAULT_FONT    &GUI_Font6x8
#endif

#ifndef GUI_DEFAULT_CURSOR
#define GUI_DEFAULT_CURSOR  &GUI_CursorArrowM
#endif

#ifndef GUI_DEFAULT_BKCOLOR
#define GUI_DEFAULT_BKCOLOR GUI_BLACK
#endif

#ifndef GUI_DEFAULT_COLOR
#define GUI_DEFAULT_COLOR   GUI_WHITE
#endif

typedef enum { GUI_WRAPMODE_NONE, GUI_WRAPMODE_WORD, GUI_WRAPMODE_CHAR } GUI_WRAPMODE;

typedef GUI_HMEM GUI_USAGE_Handle;
typedef struct tsUSAGE_APIList tUSAGE_APIList;
typedef struct GUI_Usage GUI_USAGE;
#define GUI_USAGE_h GUI_USAGE_Handle

typedef GUI_USAGE_h tUSAGE_CreateCompatible(GUI_USAGE* p);
typedef void        tUSAGE_AddPixel        (GUI_USAGE* p, int x, int y);
typedef void        tUSAGE_AddHLine        (GUI_USAGE* p, int x0, int y0, int len);
typedef void        tUSAGE_Clear           (GUI_USAGE* p);
typedef void        tUSAGE_Delete          (GUI_USAGE_h h);
typedef int         tUSAGE_GetNextDirty    (GUI_USAGE* p, int *pxOff, int yOff);

void GUI_USAGE_DecUseCnt(GUI_USAGE_Handle  hUsage);

GUI_USAGE_Handle GUI_USAGE_BM_Create(int x0, int y0, int xsize, int ysize, int Flags);
void    GUI_USAGE_Select(GUI_USAGE_Handle hUsage);
void    GUI_USAGE_AddRect(GUI_USAGE*  pUsage, int x0, int y0, int xSize, int ySize);
#define GUI_USAGE_AddPixel(p, x,y)            p->pAPI->pfAddPixel(p,x,y)
#define GUI_USAGE_AddHLine(p,x,y,len)         p->pAPI->pfAddHLine(p,x,y,len)
#define GUI_USAGE_Clear(p)                    p->pAPI->pfClear(p)
#define GUI_USAGE_Delete(p)                   p->pAPI->pfDelete(p)
#define GUI_USAGE_GetNextDirty(p,pxOff, yOff) p->pAPI->pfGetNextDirty(p,pxOff, yOff)

struct tsUSAGE_APIList {
  tUSAGE_AddPixel*                pfAddPixel;
  tUSAGE_AddHLine*                pfAddHLine;
  tUSAGE_Clear*                   pfClear;
  tUSAGE_CreateCompatible*        pfCreateCompatible;
  tUSAGE_Delete*                  pfDelete;
  tUSAGE_GetNextDirty*            pfGetNextDirty;
} ;

struct GUI_Usage {
  int16_t x0, y0, XSize, YSize;
  const tUSAGE_APIList *pAPI;
  int16_t UseCnt;
};

#if GUI_SUPPORT_MEMDEV
  typedef struct /*GUI_MEMDEV*/ {
    int16_t                   x0, y0, XSize, YSize;
    unsigned               BytesPerLine;
    unsigned               BitsPerPixel;
    GUI_HMEM               hUsage;
    const tLCDDEV_APIList* pAPIList;
  } GUI_MEMDEV;


  void         GUI_MEMDEV__CopyFromLCD (GUI_MEMDEV_Handle hMem);
  void         GUI_MEMDEV__GetRect     (GUI_RECT* pRect);

  GUI_MEMDEV_Handle GUI_MEMDEV__CreateFixed(int x0, int y0, int xsize, int ysize, int Flags
                                          ,const tLCDDEV_APIList * pMemDevAPI);

#endif

#define LCD_HL_DrawHLine LCD_DrawHLine
#define LCD_HL_DrawPixel LCD_DrawPixel

#define GUI_ZEROINIT(Obj) GUI_MEMSET(Obj, 0, sizeof(Obj))

#define GUI_UC__GetCharSize(sText)  GUI_Context.pUC_API->pfGetCharSize(sText)
#define GUI_UC__GetCharCode(sText)  GUI_Context.pUC_API->pfGetCharCode(sText)

int   GUI_UC__CalcSizeOfChar   (uint16_t Char);
uint16_t   GUI_UC__GetCharCodeInc   (const char GUI_UNI_PTR ** ps);
int   GUI_UC__NumChars2NumBytes(const char GUI_UNI_PTR * s, int NumChars);
int   GUI_UC__NumBytes2NumChars(const char GUI_UNI_PTR * s, int NumBytes);

int  GUI__GetLineNumChars  (const char GUI_UNI_PTR *s, int MaxNumChars);
int  GUI__GetNumChars      (const char GUI_UNI_PTR *s);
int  GUI__GetLineDistX     (const char GUI_UNI_PTR *s, int Len);
int  GUI__GetFontSizeY     (void);
int  GUI__HandleEOLine     (const char GUI_UNI_PTR **ps);
void GUI__DispLine         (const char GUI_UNI_PTR *s, int Len, const GUI_RECT* pr);
void GUI__AddSpaceHex      (uint32_t v, uint8_t Len, char**ps);
void GUI__CalcTextRect     (const char GUI_UNI_PTR* pText, const GUI_RECT* pTextRectIn, GUI_RECT* pTextRectOut, int TextAlign);

int  GUI__WrapGetNumCharsDisp       (const char GUI_UNI_PTR * pText, int xSize, GUI_WRAPMODE WrapMode);
int  GUI__WrapGetNumCharsToNextLine (const char GUI_UNI_PTR * pText, int xSize, GUI_WRAPMODE WrapMode);
int  GUI__WrapGetNumBytesToNextLine (const char GUI_UNI_PTR * pText, int xSize, GUI_WRAPMODE WrapMode);
void GUI__memset    (uint8_t * p, uint8_t Fill, int NumBytes);
void GUI__memset16  (uint16_t* p, uint16_t Fill, int NumWords);
int  GUI__strlen    (const char GUI_UNI_PTR * s);
int  GUI__strcmp    (const char GUI_UNI_PTR * s0, const char GUI_UNI_PTR * s1);

void GL_DispChar         (uint16_t c);
void GL_DrawBitmap       (const GUI_BITMAP GUI_UNI_PTR * pBM, int x0, int y0);

/************************************************************
*
*                 Callback pointers for dynamic linkage
*
*************************************************************
Dynamic linkage pointers reduces configuration hassles.
*/
typedef int  GUI_tfTimer(void);
typedef char GUI_CURSOR_tfTempHide  (const GUI_RECT* pRect);
typedef void GUI_CURSOR_tfTempUnhide(void);
typedef int  WM_tfHandlePID(void);

extern const uint8_t  GUI_Pixels_ArrowS[45];
extern const uint8_t  GUI_Pixels_ArrowM[60];
extern const uint8_t  GUI_Pixels_ArrowL[150];
extern const uint8_t  GUI_Pixels_CrossS[33];
extern const uint8_t  GUI_Pixels_CrossM[126];
extern const uint8_t  GUI_Pixels_CrossL[248];
extern const uint8_t  GUI_PixelsHeaderM[5 * 17];

extern const GUI_LOGPALETTE GUI_CursorPal;
extern const GUI_LOGPALETTE GUI_CursorPalI;

extern GUI_RECT  GUI_RectDispString; /* Used by LCD_Rotate...() and GUI_DispStringInRect() */

int       GUI_GetBitmapPixelIndex(const GUI_BITMAP GUI_UNI_PTR * pBMP, unsigned x, unsigned y);
RGB_COLOR GUI_GetBitmapPixelColor(const GUI_BITMAP GUI_UNI_PTR * pBMP, unsigned x, unsigned y);

#if defined(__cplusplus)
}
#endif

#ifdef  GL_CORE_C
#define GUI_EXTERN
#else
#if defined(__cplusplus)
#define GUI_EXTERN extern "C"
#else
#define GUI_EXTERN extern
#endif
#endif

GUI_EXTERN GUI_SADDR GUI_CONTEXT        GUI_Context;        /* Thread wide globals */
GUI_EXTERN           GUI_tfTimer*       GUI_pfTimerExec;
GUI_EXTERN           WM_tfHandlePID*    WM_pfHandlePID;
GUI_EXTERN      void (*GUI_pfDispCharStyle)(uint16_t Char);

#if GUI_SUPPORT_CURSOR
  GUI_EXTERN GUI_CURSOR_tfTempHide*   GUI_CURSOR_pfTempHide;
  GUI_EXTERN GUI_CURSOR_tfTempUnhide* GUI_CURSOR_pfTempUnhide;
#endif

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

extern const GUI_UC_ENC_APILIST GUI__API_TableNone;

#if defined(__cplusplus)
  }
#endif

#undef GUI_EXTERN
