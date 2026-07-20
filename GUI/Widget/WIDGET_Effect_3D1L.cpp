#include "WIDGET.h"

/*********************************************************************
*
*       _WIDGET_EFFECT_3D1L_DrawUpRect
*
* Add. info:
*   This routine does not preserve the drawing colors for speed
*   reasons. If this is required, it should be done in the calling
*   routine.
*/
static void _WIDGET_EFFECT_3D1L_DrawUpRect(GUI_RECT r) {
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
	_WIDGET_EFFECT_3D1L_DrawUpRect(WM_GetClientRect());
}
static void _WIDGET_EFFECT_3D1L_DrawDownRect(GUI_RECT r) {
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
	_WIDGET_EFFECT_3D1L_DrawDownRect(WM_GetClientRect());
}
static GUI_RECT _WIDGET_EFFECT_3D1L_GetRect() {
	return WM_GetClientRect() - 1;
}
const WIDGET_EFFECT WIDGET_Effect_3D1L{
	_WIDGET_EFFECT_3D1L_DrawUp,
	_WIDGET_EFFECT_3D1L_DrawDown,
	_WIDGET_EFFECT_3D1L_DrawUpRect,
	_WIDGET_EFFECT_3D1L_DrawDownRect,
	_WIDGET_EFFECT_3D1L_GetRect,
	1
};
