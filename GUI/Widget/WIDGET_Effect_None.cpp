
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

static void _WIDGET_EFFECT_None_DrawDown(void) {
}
static void _WIDGET_EFFECT_None_DrawUp(void) {
}
static void _WIDGET_EFFECT_None_DrawDownRect(GUI_RECT r) {
  GUI_USE_PARA(r);
}
static void _WIDGET_EFFECT_None_DrawUpRect(GUI_RECT r) {
  GUI_USE_PARA(r);
}
static GUI_RECT _WIDGET_EFFECT_None_GetRect() {
  return WM_GetClientRect();
}
void WIDGET_SetDefaultEffect_None(void) {
  WIDGET_SetDefaultEffect(&WIDGET_Effect_None);
}
const WIDGET_EFFECT WIDGET_Effect_None = {
  _WIDGET_EFFECT_None_DrawUp,
  _WIDGET_EFFECT_None_DrawDown,
  _WIDGET_EFFECT_None_DrawUpRect,
  _WIDGET_EFFECT_None_DrawDownRect,
  _WIDGET_EFFECT_None_GetRect,
  0
};
