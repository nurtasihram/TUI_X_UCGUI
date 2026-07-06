#pragma once
#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef WM_HMEM DROPDOWN_Handle;

#define DROPDOWN_CF_AUTOSCROLLBAR   (1 << 0)
#define DROPDOWN_CF_UP              (1 << 1)
#define DROPDOWN_CI_UNSEL    0
#define DROPDOWN_CI_SEL      1
#define DROPDOWN_CI_SELFOCUS 2
#define DROPDOWN_EnableMemdev(hObj)  WM_EnableMemdev(hObj)
#define DROPDOWN_DisableMemdev(hObj) WM_DisableMemdev(hObj)
#define DROPDOWN_Delete(hObj)        WM_DeleteWindow(hObj)
#define DROPDOWN_Paint(hObj)         WM_Paint(hObj)
#define DROPDOWN_Invalidate(hObj)    WM_Invalidate(hObj)
DROPDOWN_Handle DROPDOWN_Create        (WM_HWIN hWinParent, int x0, int y0, int xsize, int ysize, int Flags);
DROPDOWN_Handle DROPDOWN_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
DROPDOWN_Handle DROPDOWN_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                        int WinFlags, int ExFlags, int Id);
void     DROPDOWN_AddKey          (DROPDOWN_Handle hObj, int Key);
void     DROPDOWN_AddString       (DROPDOWN_Handle hObj, const char* s);
void     DROPDOWN_Collapse        (DROPDOWN_Handle hObj);
void     DROPDOWN_DecSel          (DROPDOWN_Handle hObj);
void     DROPDOWN_DeleteItem      (DROPDOWN_Handle hObj, unsigned int Index);
void     DROPDOWN_Expand          (DROPDOWN_Handle hObj);
unsigned DROPDOWN_GetItemSpacing  (DROPDOWN_Handle hObj);
int      DROPDOWN_GetNumItems     (DROPDOWN_Handle hObj);
int      DROPDOWN_GetSel          (DROPDOWN_Handle hObj);
void     DROPDOWN_IncSel          (DROPDOWN_Handle hObj);
void     DROPDOWN_InsertString    (DROPDOWN_Handle hObj, const char* s, unsigned int Index);
void     DROPDOWN_SetAutoScroll   (DROPDOWN_Handle hObj, int OnOff);
void     DROPDOWN_SetBkColor      (DROPDOWN_Handle hObj, unsigned int index, RGB_COLOR color);
void     DROPDOWN_SetFont         (DROPDOWN_Handle hObj, const GUI_FONT  * pfont);
void     DROPDOWN_SetItemSpacing  (DROPDOWN_Handle hObj, unsigned Value);
void     DROPDOWN_SetSel          (DROPDOWN_Handle hObj, int Sel);
void     DROPDOWN_SetScrollbarWidth(DROPDOWN_Handle hObj, unsigned Width);
void     DROPDOWN_SetTextAlign    (DROPDOWN_Handle hObj, int Align);
void     DROPDOWN_SetTextColor    (DROPDOWN_Handle hObj, unsigned int index, RGB_COLOR color);
void     DROPDOWN_SetTextHeight   (DROPDOWN_Handle hObj, unsigned TextHeight);
const GUI_FONT  * DROPDOWN_GetDefaultFont(void);
void            DROPDOWN_SetDefaultFont(const GUI_FONT* pFont);

#if defined(__cplusplus)
  }
#endif
