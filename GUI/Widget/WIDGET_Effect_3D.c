
#include <stdlib.h>
#include <string.h>
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

/*********************************************************************
*
*       _WIDGET_EFFECT_3D_DrawUpRect
*
* Add. info:
*   This routine does not preserve the drawing colors for speed
*   reasons. If this is required, it should be done in the calling
*   routine.
*/
static void _WIDGET_EFFECT_3D_DrawUpRect(const GUI_RECT* pRect) {
  GUI_RECT r;
/* Saving of context no longer required ... Speeds up the system
  GUI_CONTEXT Context;
  GUI_SaveContext(&Context);
*/
  r = *pRect;
  GUI_SetColor(RGB_BLACK);
  GUI_DrawRect(r.x0, r.y0, r.x1, r.y1);          /* Draw rectangle around it */
 /* Draw the bright sides */
  GUI_SetColor(RGB_WHITE);
  GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 2);    /* Draw top line */
  GUI_DrawVLine(r.x0 + 1, r.y0 + 1, r.y1 - 2);
  /* Draw the dark sides */
  GUI_SetColor(RGB_GRAYL(0x55));
  GUI_DrawHLine(r.y1-1, r.x0 + 1, r.x1 - 1);
  GUI_DrawVLine(r.x1-1, r.y0 + 1, r.y1 - 2);
/*  GUI_RestoreContext(&Context); */
}
static void _WIDGET_EFFECT_3D_DrawDownRect(const GUI_RECT* pRect) {
  GUI_RECT r;
  r = *pRect;
  GUI_SetColor(RGB_BLACK);  /* TBD: Use halftone */
/*  GUI_DrawRect(0, 0, r.x1, r.y1);*/
  /* Draw the upper left sides */
  GUI_SetColor(RGB_GRAYL(0x80));
  GUI_DrawHLine(r.y0, r.x0, r.x1);
  GUI_DrawVLine(r.x0, r.y0 + 1, r.y1);
  GUI_SetColor(0x0);
  GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 1);
  GUI_DrawVLine(r.x0 + 1, r.y0 + 2, r.y1 - 1);
  /* Draw the lower right sides */
  GUI_SetColor(RGB_WHITE);
  GUI_DrawHLine(r.y1, r.x0 + 1, r.x1);
  GUI_DrawVLine(r.x1, r.y0 + 1, r.y1);
  GUI_SetColor(RGB_GRAYL(0xc0));
  GUI_DrawHLine(r.y1 - 1, r. x0 + 2, r.x1-1);
  GUI_DrawVLine(r.x1 - 1, r. y0 + 2, r.y1-1);
}
static void _WIDGET_EFFECT_3D_DrawDown(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _WIDGET_EFFECT_3D_DrawDownRect(&r);
}
static void _WIDGET_EFFECT_3D_GetRect(GUI_RECT* pRect) {
  WM_GetClientRect(pRect);
  GUI__ReduceRect(pRect, pRect, 2);
}
void WIDGET_EFFECT_3D_DrawUp(void) {
  GUI_RECT r;
  WM_GetClientRect(&r);
  _WIDGET_EFFECT_3D_DrawUpRect(&r);
}
void WIDGET_SetDefaultEffect_3D(void) {
  WIDGET_SetDefaultEffect(&WIDGET_Effect_3D);
}
const WIDGET_EFFECT WIDGET_Effect_3D = {
  WIDGET_EFFECT_3D_DrawUp,
  _WIDGET_EFFECT_3D_DrawDown,
  _WIDGET_EFFECT_3D_DrawUpRect,
  _WIDGET_EFFECT_3D_DrawDownRect,
  _WIDGET_EFFECT_3D_GetRect,
  2
};


