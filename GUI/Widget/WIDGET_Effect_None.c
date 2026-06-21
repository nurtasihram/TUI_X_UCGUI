
#include <stdlib.h>
#include <string.h>
#include "WIDGET.h"
#include "GUIDebug.h"
#include "GUI.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

static void _WIDGET_EFFECT_None_DrawDown(void) {
}
static void _WIDGET_EFFECT_None_DrawUp(void) {
}
static void _WIDGET_EFFECT_None_DrawDownRect(const GUI_RECT* pRect) {
  GUI_USE_PARA(pRect);
}
static void _WIDGET_EFFECT_None_DrawUpRect(const GUI_RECT* pRect) {
  GUI_USE_PARA(pRect);
}
static void _WIDGET_EFFECT_None_GetRect(GUI_RECT * pRect) {
  WM_GetClientRect(pRect);
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
 /* GUI_WINSUPPORT */
/*************************** End of file ****************************/
