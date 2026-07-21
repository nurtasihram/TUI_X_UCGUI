#pragma once
#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

enum CHECKBOX_BI {
	 CHECKBOX_BI_INACTIV = 0,
	 CHECKBOX_BI_ACTIV,
	 CHECKBOX_BI_INACTIV_3STATE,
	 CHECKBOX_BI_ACTIV_3STATE
};
enum CHECKBOX_CI {
	 CHECKBOX_CI_INACTIV = 0,
	 CHECKBOX_CI_ACTIV
};

typedef WM_Obj * CHECKBOX_Handle;
CHECKBOX_Handle CHECKBOX_Create        (int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int Flags);
CHECKBOX_Handle CHECKBOX_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK* cb);
CHECKBOX_Handle CHECKBOX_CreateEx      (int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
                                        int WinFlags, int ExFlags, int Id);
int  CHECKBOX_GetState    (CHECKBOX_Handle hObj);
int  CHECKBOX_IsChecked   (CHECKBOX_Handle hObj);
void CHECKBOX_SetBkColor  (CHECKBOX_Handle hObj, RGBC Color);
void CHECKBOX_SetFont     (CHECKBOX_Handle hObj, PCFONT pFont);
void CHECKBOX_SetImage    (CHECKBOX_Handle hObj, PCBITMAP pBitmap, unsigned int Index);
void CHECKBOX_SetNumStates(CHECKBOX_Handle hObj, unsigned NumStates);
void CHECKBOX_SetSpacing  (CHECKBOX_Handle hObj, unsigned Spacing);
void CHECKBOX_SetState    (CHECKBOX_Handle hObj, unsigned State);
void CHECKBOX_SetText     (CHECKBOX_Handle hObj, const char * pText);
void CHECKBOX_SetTextAlign(CHECKBOX_Handle hObj, int Align);
void CHECKBOX_SetTextColor(CHECKBOX_Handle hObj, RGBC Color);
