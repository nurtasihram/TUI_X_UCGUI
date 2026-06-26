
#include <stdlib.h>
#include <string.h>
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

/*********************************************************************
*
*       _WIDGET_EFFECT_3D1L_DrawUpRect
*
* Add. info:
*   This routine does not preserve the drawing colors for speed
*   reasons. If this is required, it should be done in the calling
*   routine.
*/
static void _WIDGET_EFFECT_3D1L_DrawUpRect(const GUI_RECT* pRect) {
  GUI_RECT r;
  r = *pRect;
  /* Draw the upper left sides */
  GUI_SetColor(RGB_GRAYL(0xE7));
  GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
  GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
  /* Draw the lower right sides */
  GUI_SetColor(RGB_GRAYL(0x60));
  GUI_DrawHLine(r.y1, r.x0, r.x1);
  GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
}
static void _WIDGET_EFFECT_3D1L_DrawUp(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _WIDGET_EFFECT_3D1L_DrawUpRect(&r);
}
static void _WIDGET_EFFECT_3D1L_DrawDownRect(const GUI_RECT* pRect) {
  GUI_RECT r;
  r = *pRect;
  /* Draw the upper left sides */
  GUI_SetColor(RGB_GRAYL(0x60));
  GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
  GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
  /* Draw the lower right sides */
  GUI_SetColor(RGB_GRAYL(0xE7));
  GUI_DrawHLine(r.y1, r.x0, r.x1);
  GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
}
static void _WIDGET_EFFECT_3D1L_DrawDown(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _WIDGET_EFFECT_3D1L_DrawDownRect(&r);
}
static void _WIDGET_EFFECT_3D1L_GetRect(GUI_RECT* pRect) {
  WM_GetClientRect(pRect);
  GUI__ReduceRect(pRect, pRect, 1);
}
void WIDGET_SetDefaultEffect_3D1L(void) {
  WIDGET_SetDefaultEffect(&WIDGET_Effect_3D1L);
}
const WIDGET_EFFECT WIDGET_Effect_3D1L = {
  _WIDGET_EFFECT_3D1L_DrawUp,
  _WIDGET_EFFECT_3D1L_DrawDown,
  _WIDGET_EFFECT_3D1L_DrawUpRect,
  _WIDGET_EFFECT_3D1L_DrawDownRect,
  _WIDGET_EFFECT_3D1L_GetRect,
  1
};


