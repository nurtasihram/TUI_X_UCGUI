#pragma once

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

typedef WM_HMEM PROGBAR_Handle;

PROGBAR_Handle PROGBAR_Create        (int x0, int y0, int xsize, int ysize, int Flags);
PROGBAR_Handle PROGBAR_CreateAsChild (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags);
PROGBAR_Handle PROGBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
PROGBAR_Handle PROGBAR_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                      int WinFlags, int ExFlags, int Id);

                                      void PROGBAR_SetBarColor (PROGBAR_Handle hObj, unsigned int index, RGBC color);
void PROGBAR_SetFont     (PROGBAR_Handle hObj, PCFONT pfont);
void PROGBAR_SetMinMax   (PROGBAR_Handle hObj, int Min, int Max);
void PROGBAR_SetText     (PROGBAR_Handle hObj, const char* s);
void PROGBAR_SetTextAlign(PROGBAR_Handle hObj, int Align);
void PROGBAR_SetTextColor(PROGBAR_Handle hObj, unsigned int index, RGBC color);
void PROGBAR_SetTextPos  (PROGBAR_Handle hObj, int XOff, int YOff);
void PROGBAR_SetValue    (PROGBAR_Handle hObj, int v);
