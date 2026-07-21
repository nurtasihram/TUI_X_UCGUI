#pragma once

#include "WM.h"
#include "DIALOG.h"      /* Req. for Create indirect data structure */
#include "HEADER.h"

#define LISTVIEW_CI_UNSEL    0
#define LISTVIEW_CI_SEL      1
#define LISTVIEW_CI_SELFOCUS 2

typedef WM_HMEM LISTVIEW_Handle;

LISTVIEW_Handle LISTVIEW_Create        (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int SpecialFlags);
LISTVIEW_Handle LISTVIEW_CreateAttached(WM_HWIN hParent, int Id, int SpecialFlags);
LISTVIEW_Handle LISTVIEW_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
LISTVIEW_Handle LISTVIEW_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                        int WinFlags, int ExFlags, int Id);

void             LISTVIEW_AddColumn     (LISTVIEW_Handle hObj, int Width, const char * s, int Align);
void             LISTVIEW_AddRow        (LISTVIEW_Handle hObj, const GUI_ConstString * ppText);
void             LISTVIEW_DecSel        (LISTVIEW_Handle hObj);
void             LISTVIEW_DeleteColumn  (LISTVIEW_Handle hObj, unsigned Index);
void             LISTVIEW_DeleteRow     (LISTVIEW_Handle hObj, unsigned Index);
RGBC        LISTVIEW_GetBkColor    (LISTVIEW_Handle hObj, unsigned Index);
PCFONT LISTVIEW_GetFont       (LISTVIEW_Handle hObj);
HEADER_Handle    LISTVIEW_GetHeader     (LISTVIEW_Handle hObj);
unsigned         LISTVIEW_GetNumColumns (LISTVIEW_Handle hObj);
unsigned         LISTVIEW_GetNumRows    (LISTVIEW_Handle hObj);
int              LISTVIEW_GetSel        (LISTVIEW_Handle hObj);
RGBC        LISTVIEW_GetTextColor  (LISTVIEW_Handle hObj, unsigned Index);
void             LISTVIEW_IncSel        (LISTVIEW_Handle hObj);
void             LISTVIEW_SetBkColor    (LISTVIEW_Handle hObj, unsigned int Index, RGBC Color);
void             LISTVIEW_SetColumnWidth(LISTVIEW_Handle hObj, unsigned int Index, int Width);
void             LISTVIEW_SetFont       (LISTVIEW_Handle hObj, PCFONT pFont);
int              LISTVIEW_SetGridVis    (LISTVIEW_Handle hObj, int Show);
void             LISTVIEW_SetItemBkColor(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index, RGBC Color);
void             LISTVIEW_SetItemText   (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, const char * s);
void             LISTVIEW_SetItemTextColor(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index, RGBC Color);
void             LISTVIEW_SetLBorder    (LISTVIEW_Handle hObj, unsigned BorderSize);
void             LISTVIEW_SetRBorder    (LISTVIEW_Handle hObj, unsigned BorderSize);
unsigned         LISTVIEW_SetRowHeight  (LISTVIEW_Handle hObj, unsigned RowHeight);
void             LISTVIEW_SetSel        (LISTVIEW_Handle hObj, int Sel);
void             LISTVIEW_SetTextAlign  (LISTVIEW_Handle hObj, unsigned int Index, int Align);
void             LISTVIEW_SetTextColor  (LISTVIEW_Handle hObj, unsigned int Index, RGBC Color);
