module;

#include "WIDGET.h"      /* Req. for Create indirect data structure */
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */

export module TUX.Widget.Slider;

export {

constexpr uint16_t SLIDER_STATE_PRESSED    = WIDGET_STATE_USER0;
constexpr uint16_t SLIDER_CF_VERTICAL = WIDGET_CF_VERTICAL;

typedef WM_Obj * SLIDER_Handle;

SLIDER_Handle SLIDER_Create        (int x0, int y0, int xsize, int ysize, WM_Obj * hParent, int Id, int WinFlags, int SpecialFlags);
SLIDER_Handle SLIDER_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
SLIDER_Handle SLIDER_CreateEx      (int x0, int y0, int xsize, int ysize, WM_Obj * hParent,
									int WinFlags, int ExFlags, int Id);

void SLIDER_Inc         (SLIDER_Handle hObj);
void SLIDER_Dec         (SLIDER_Handle hObj);
void SLIDER_SetBkColor  (SLIDER_Handle hObj, RGBC Color);
void SLIDER_SetWidth    (SLIDER_Handle hObj, int Width);
void SLIDER_SetValue    (SLIDER_Handle hObj, int v);
void SLIDER_SetRange    (SLIDER_Handle hObj, int Min, int Max);
void SLIDER_SetNumTicks (SLIDER_Handle hObj, int NumTicks);
int  SLIDER_GetValue(SLIDER_Handle hObj);

}
