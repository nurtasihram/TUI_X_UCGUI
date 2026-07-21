#pragma once
#include "WM.h"
#include "DIALOG.h"      /* Req. for Create indirect data structure */
#define MULTIPAGE_ALIGN_LEFT    (0<<0)
#define MULTIPAGE_ALIGN_RIGHT   (1<<0)
#define MULTIPAGE_ALIGN_TOP     (0<<2)
#define MULTIPAGE_ALIGN_BOTTOM  (1<<2)
typedef WM_Obj * MULTIPAGE_Handle;
MULTIPAGE_Handle MULTIPAGE_Create        (int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int Flags, int SpecialFlags);
MULTIPAGE_Handle MULTIPAGE_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK* cb);
MULTIPAGE_Handle MULTIPAGE_CreateEx      (int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
                                          int WinFlags, int ExFlags, int Id);
/* Methods changing properties */
void        MULTIPAGE_AddPage       (MULTIPAGE_Handle hObj, WM_Obj * hWin ,const char* pText);
void        MULTIPAGE_DeletePage    (MULTIPAGE_Handle hObj, unsigned Index, int Delete);
void        MULTIPAGE_SelectPage    (MULTIPAGE_Handle hObj, unsigned Index);
void        MULTIPAGE_EnablePage    (MULTIPAGE_Handle hObj, unsigned Index);
void        MULTIPAGE_DisablePage   (MULTIPAGE_Handle hObj, unsigned Index);
void        MULTIPAGE_SetText       (MULTIPAGE_Handle hObj, const char* pText, unsigned Index);
void        MULTIPAGE_SetBkColor    (MULTIPAGE_Handle hObj, RGBC Color, unsigned Index);
void        MULTIPAGE_SetTextColor  (MULTIPAGE_Handle hObj, RGBC Color, unsigned Index);
void        MULTIPAGE_SetFont       (MULTIPAGE_Handle hObj, PCFONT pFont);
void        MULTIPAGE_SetAlign      (MULTIPAGE_Handle hObj, unsigned Align);
int         MULTIPAGE_GetSelection  (MULTIPAGE_Handle hObj);
WM_Obj *     MULTIPAGE_GetWindow     (MULTIPAGE_Handle hObj, unsigned Index);
int         MULTIPAGE_IsPageEnabled (MULTIPAGE_Handle hObj, unsigned Index);
