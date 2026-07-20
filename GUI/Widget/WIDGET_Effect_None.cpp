#include "WIDGET.h"

static void _WIDGET_EFFECT_None_DrawDown(void) {}
static void _WIDGET_EFFECT_None_DrawUp(void) {}
static void _WIDGET_EFFECT_None_DrawDownRect(GUI_RECT) {}
static void _WIDGET_EFFECT_None_DrawUpRect(GUI_RECT) {}
static GUI_RECT _WIDGET_EFFECT_None_GetRect() {
	return WM_GetClientRect();
}
const WIDGET_EFFECT WIDGET_Effect_None{
	_WIDGET_EFFECT_None_DrawUp,
	_WIDGET_EFFECT_None_DrawDown,
	_WIDGET_EFFECT_None_DrawUpRect,
	_WIDGET_EFFECT_None_DrawDownRect,
	_WIDGET_EFFECT_None_GetRect,
	0
};
