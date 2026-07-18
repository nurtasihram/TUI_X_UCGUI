
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

static void _WIDGET_EFFECT_Simple_DrawUpRect(GUI_RECT r) {
  GUI_CONTEXT Context;
  GUI_SaveContext(&Context);
  GUI_SetColor(RGB_BLACK);
  GUI_DrawRect(r);          /* Draw rectangle around it */
  GUI_RestoreContext(&Context);
}
static void _WIDGET_EFFECT_Simple_DrawUp(void) {
  _WIDGET_EFFECT_Simple_DrawUpRect(WM_GetClientRect());
}
static void _WIDGET_EFFECT_Simple_DrawDownRect(GUI_RECT r) {
  GUI_CONTEXT Context;
  GUI_SaveContext(&Context);
  GUI_SetColor(RGB_BLACK);
  GUI_DrawRect(r);          /* Draw rectangle around it */
  GUI_RestoreContext(&Context);
}
static void _WIDGET_EFFECT_Simple_DrawDown(void) {
  _WIDGET_EFFECT_Simple_DrawDownRect(WM_GetClientRect());
}
static GUI_RECT _WIDGET_EFFECT_Simple_GetRect() {
  GUI_RECT Rect = WM_GetClientRect();
  GUI__ReduceRect(&Rect, &Rect, 1);
  return Rect;
}
void WIDGET_SetDefaultEffect_Simple(void) {
  WIDGET_SetDefaultEffect(&WIDGET_Effect_Simple);
}
const WIDGET_EFFECT WIDGET_Effect_Simple = {
  _WIDGET_EFFECT_Simple_DrawUp,
  _WIDGET_EFFECT_Simple_DrawDown,
  _WIDGET_EFFECT_Simple_DrawUpRect,
  _WIDGET_EFFECT_Simple_DrawDownRect,
  _WIDGET_EFFECT_Simple_GetRect,
  1
};
