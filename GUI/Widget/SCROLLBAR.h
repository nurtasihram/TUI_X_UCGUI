#pragma once

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"

#define SCROLLBAR_STATE_PRESSED    WIDGET_STATE_USER0
#define SCROLLBAR_CF_VERTICAL     WIDGET_CF_VERTICAL
#define SCROLLBAR_CF_FOCUSSABLE   WIDGET_STATE_FOCUSSABLE

typedef WM_Obj * SCROLLBAR_Handle;

SCROLLBAR_Handle SCROLLBAR_Create        (int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int WinFlags, int SpecialFlags);
SCROLLBAR_Handle SCROLLBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK* cb);
SCROLLBAR_Handle SCROLLBAR_CreateAttached(WM_Obj * hParent, int SpecialFlags);
SCROLLBAR_Handle SCROLLBAR_CreateEx      (int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
                                          int WinFlags, int ExFlags, int Id);

void SCROLLBAR_AddValue    (SCROLLBAR_Handle hObj, int Add);
void SCROLLBAR_Dec         (SCROLLBAR_Handle hObj);
void SCROLLBAR_Inc         (SCROLLBAR_Handle hObj);
void SCROLLBAR_SetNumItems (SCROLLBAR_Handle hObj, int NumItems);
void SCROLLBAR_SetPageSize (SCROLLBAR_Handle hObj, int PageSize);
void SCROLLBAR_SetValue    (SCROLLBAR_Handle hObj, int v);
int  SCROLLBAR_SetWidth    (SCROLLBAR_Handle hObj, int Width);
void SCROLLBAR_SetState    (SCROLLBAR_Handle hObj, const WM_SCROLL_STATE* pState);
int SCROLLBAR_GetValue(SCROLLBAR_Handle hObj);
