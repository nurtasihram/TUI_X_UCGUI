#pragma once
#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"      /* Req. for Create indirect data structure */
#define SLIDER_STATE_PRESSED    WIDGET_STATE_USER0
#define SLIDER_CF_VERTICAL WIDGET_CF_VERTICAL
/*********************************************************************
*
*                         Public Types
*
**********************************************************************
*/
typedef WM_HMEM SLIDER_Handle;
SLIDER_Handle SLIDER_Create        (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int WinFlags, int SpecialFlags);
SLIDER_Handle SLIDER_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
SLIDER_Handle SLIDER_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                    int WinFlags, int ExFlags, int Id);
/* Methods changing properties */
/* Note: These are just examples. The actual methods available for the
   widget will depend on the type of widget. */
void SLIDER_Inc         (SLIDER_Handle hObj);
void SLIDER_Dec         (SLIDER_Handle hObj);
void SLIDER_SetBkColor  (SLIDER_Handle hObj, RGB_COLOR Color);
void SLIDER_SetWidth    (SLIDER_Handle hObj, int Width);
void SLIDER_SetValue    (SLIDER_Handle hObj, int v);
void SLIDER_SetRange    (SLIDER_Handle hObj, int Min, int Max);
void SLIDER_SetNumTicks (SLIDER_Handle hObj, int NumTicks);
void SLIDER_SetDefaultBkColor(RGB_COLOR Color);
int SLIDER_GetValue(SLIDER_Handle hObj);
