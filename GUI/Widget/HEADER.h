#pragma once
#include "GUI.h"
#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
typedef WM_HMEM HEADER_Handle;
HEADER_Handle HEADER_Create        (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int SpecialFlags);
HEADER_Handle HEADER_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);
HEADER_Handle HEADER_CreateAttached(WM_HWIN hParent, int Id, int SpecialFlags);
HEADER_Handle HEADER_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                    int WinFlags, int ExFlags, int Id);
#define HEADER_EnableMemdev(hObj)  WM_EnableMemdev    (hObj)
#define HEADER_DisableMemdev(hObj) WM_DisableMemdev   (hObj)
#define HEADER_Delete(hObj)        WM_DeleteWindow    (hObj)
#define HEADER_Paint(hObj)         WM_Paint           (hObj)
#define HEADER_Invalidate(hObj)    WM_Invalidate(hObj)
/* Set defaults */
RGB_COLOR          HEADER_SetDefaultBkColor  (RGB_COLOR Color);
const GUI_CURSOR  * HEADER_SetDefaultCursor   (const GUI_CURSOR * pCursor);
const GUI_FONT  *   HEADER_SetDefaultFont     (const GUI_FONT  * pFont);
int                HEADER_SetDefaultBorderH (int Spacing);
int                HEADER_SetDefaultBorderV (int Spacing);
RGB_COLOR          HEADER_SetDefaultTextColor(RGB_COLOR Color);
/* Get defaults */
RGB_COLOR          HEADER_GetDefaultBkColor  (void);
const GUI_CURSOR  * HEADER_GetDefaultCursor   (void);
const GUI_FONT  *   HEADER_GetDefaultFont     (void);
int                HEADER_GetDefaultBorderH  (void);
int                HEADER_GetDefaultBorderV  (void);
RGB_COLOR          HEADER_GetDefaultTextColor(void);
void HEADER_AddItem            (HEADER_Handle hObj, int Width, const char * s, int Align);
void HEADER_DeleteItem         (HEADER_Handle hObj, unsigned Index);
int  HEADER_GetHeight          (HEADER_Handle hObj);
int  HEADER_GetItemWidth       (HEADER_Handle hObj, unsigned int Index);
int  HEADER_GetNumItems        (HEADER_Handle hObj);
void HEADER_SetBitmap          (HEADER_Handle hObj, unsigned int Index, const GUI_BITMAP * pBitmap);
void HEADER_SetBitmapEx        (HEADER_Handle hObj, unsigned int Index, const GUI_BITMAP * pBitmap, int x, int y);
void HEADER_SetBkColor         (HEADER_Handle hObj, RGB_COLOR Color);
void HEADER_SetBMP             (HEADER_Handle hObj, unsigned int Index, const void * pBitmap);
void HEADER_SetBMPEx           (HEADER_Handle hObj, unsigned int Index, const void * pBitmap, int x, int y);
void HEADER_SetFont            (HEADER_Handle hObj, const GUI_FONT  * pFont);
void HEADER_SetHeight          (HEADER_Handle hObj, int Height);
void HEADER_SetTextAlign       (HEADER_Handle hObj, unsigned int Index, int Align);
void HEADER_SetItemText        (HEADER_Handle hObj, unsigned int Index, const char * s);
void HEADER_SetItemWidth       (HEADER_Handle hObj, unsigned int Index, int Width);
void HEADER_SetScrollPos       (HEADER_Handle hObj, int ScrollPos);
void HEADER_SetTextColor       (HEADER_Handle hObj, RGB_COLOR Color);
#ifdef HEADER_SPACING_H
#define HEADER_BORDER_H_DEFAULT HEADER_SPACING_H
#endif
#ifdef HEADER_SPACING_V
#define HEADER_BORDER_V_DEFAULT HEADER_SPACING_V
#endif
#define HEADER_SetDefaultSpacingH(Value) HEADER_SetDefaultBorderH(Value)
#define HEADER_SetDefaultSpacingV(Value) HEADER_SetDefaultBorderV(Value)
#define HEADER_GetDefaultSpacingH()      HEADER_GetDefaultBorderH()
#define HEADER_GetDefaultSpacingV()      HEADER_GetDefaultBorderV()
#if defined(__cplusplus)
  }
#endif
