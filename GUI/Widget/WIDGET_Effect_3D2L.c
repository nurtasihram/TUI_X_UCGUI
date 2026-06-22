
#include <stdlib.h>
#include <string.h>
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

/*********************************************************************
*
*       _WIDGET_EFFECT_3D2L_DrawUpRect
*
* Add. info:
*   This routine does not preserve the drawing colors for speed
*   reasons. If this is required, it should be done in the calling
*   routine.
*/
static void _WIDGET_EFFECT_3D2L_DrawUpRect(const GUI_RECT* pRect) {
  GUI_RECT r;
  r = *pRect;
  /* Draw the upper left sides */
  LCD_SetColor(RGB_GRAYL(0xD0));
  GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
  GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
  LCD_SetColor(RGB_GRAYL(0xE7));
  GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 2);
  GUI_DrawVLine(r.x0 + 1, r.y0 + 2, r.y1 - 2);
  /* Draw the lower right sides */
  LCD_SetColor(RGB_GRAYL(0x60));
  GUI_DrawHLine(r.y1, r.x0, r.x1);
  GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
  LCD_SetColor(RGB_GRAYL(0x9A));
  GUI_DrawHLine(r.y1 - 1, r.x0 + 1, r.x1 - 1);
  GUI_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 2);
}
static void _WIDGET_EFFECT_3D2L_DrawUp(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _WIDGET_EFFECT_3D2L_DrawUpRect(&r);
}
static void _WIDGET_EFFECT_3D2L_DrawDownRect(const GUI_RECT* pRect) {
  GUI_RECT r;
  r = *pRect;
  /* Draw the upper left sides */
  LCD_SetColor(RGB_GRAYL(0x9A));
  GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
  GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
  LCD_SetColor(RGB_GRAYL(0x60));
  GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 2);
  GUI_DrawVLine(r.x0 + 1, r.y0 + 2, r.y1 - 2);
  /* Draw the lower right sides */
  LCD_SetColor(RGB_GRAYL(0xE7));
  GUI_DrawHLine(r.y1, r.x0, r.x1);
  GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
  LCD_SetColor(RGB_GRAYL(0xD0));
  GUI_DrawHLine(r.y1 - 1, r.x0 + 1, r.x1 - 1);
  GUI_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 2);
}
static void _WIDGET_EFFECT_3D2L_DrawDown(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _WIDGET_EFFECT_3D2L_DrawDownRect(&r);
}
static void _WIDGET_EFFECT_3D2L_GetRect(GUI_RECT* pRect) {
  WM_GetClientRect(pRect);
  GUI__ReduceRect(pRect, pRect, 2);
}
void WIDGET_SetDefaultEffect_3D2L(void) {
  WIDGET_SetDefaultEffect(&WIDGET_Effect_3D2L);
}
const WIDGET_EFFECT WIDGET_Effect_3D2L = {
  _WIDGET_EFFECT_3D2L_DrawUp,
  _WIDGET_EFFECT_3D2L_DrawDown,
  _WIDGET_EFFECT_3D2L_DrawUpRect,
  _WIDGET_EFFECT_3D2L_DrawDownRect,
  _WIDGET_EFFECT_3D2L_GetRect,
  2
};


