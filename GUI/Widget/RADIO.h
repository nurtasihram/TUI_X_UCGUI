#pragma once
#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#define RADIO_BI_INACTIV 0
#define RADIO_BI_ACTIV   1
#define RADIO_BI_CHECK   2
/*********************************************************************
*
*            Defaults for public configuration switches
*
**********************************************************************
The following are defaults for config switches which affect the
interface specified in this module
*/
#define RADIO_TEXTPOS_RIGHT       0
#define RADIO_TEXTPOS_LEFT        WIDGET_STATE_USER0  /* Not implemented, TBD */
typedef WM_HMEM RADIO_Handle;
RADIO_Handle RADIO_Create        (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, unsigned Para);
RADIO_Handle RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK* cb);
RADIO_Handle RADIO_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                  int WinFlags, int ExFlags, int Id, int NumItems, int Spacing);
void RADIO_AddValue     (RADIO_Handle hObj, int Add);
void RADIO_Dec          (RADIO_Handle hObj);
void RADIO_Inc          (RADIO_Handle hObj);
void RADIO_SetBkColor   (RADIO_Handle hObj, RGBC Color);
void RADIO_SetFont      (RADIO_Handle hObj, PCFONT pFont);
void RADIO_SetGroupId   (RADIO_Handle hObj, uint8_t GroupId);
void RADIO_SetImage     (RADIO_Handle hObj, PCBITMAP pBitmap, unsigned int Index);
void RADIO_SetText      (RADIO_Handle hObj, const char* pText, unsigned Index);
void RADIO_SetTextColor (RADIO_Handle hObj, RGBC Color);
void RADIO_SetValue     (RADIO_Handle hObj, int v);
int RADIO_GetValue(RADIO_Handle hObj);

