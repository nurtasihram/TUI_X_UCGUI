#include "WIDGET.h"

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
	GUI_SetColor(RGB_BLACK);
	GUI_DrawRect(r);
}
static void _WIDGET_EFFECT_Simple_DrawDown(void) {
	_WIDGET_EFFECT_Simple_DrawDownRect(WM_GetClientRect());
}
static GUI_RECT _WIDGET_EFFECT_Simple_GetRect() {
	return WM_GetClientRect() - 1;
}
const WIDGET_EFFECT WIDGET_Effect_Simple = {
	_WIDGET_EFFECT_Simple_DrawUp,
	_WIDGET_EFFECT_Simple_DrawDown,
	_WIDGET_EFFECT_Simple_DrawUpRect,
	_WIDGET_EFFECT_Simple_DrawDownRect,
	_WIDGET_EFFECT_Simple_GetRect,
	1
};
