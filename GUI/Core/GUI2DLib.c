#include "GUI_Protected.h"
#include "GUIDebug.h"

void GUI_SetDefault(void) {
	GUI_SetBkColor(GUI_DEFAULT_BKCOLOR);
	GUI_SetColor(GUI_DEFAULT_COLOR);
	GUI_SetTextAlign(0);
	GUI_SetTextMode(0);
	GUI_SetDrawMode(0);
	GUI_SetFont(GUI_DEFAULT_FONT);
}

static void _DrawRect(int x0, int y0, int x1, int y1) {
	LCD_DrawHLine(x0, y0, x1);
	LCD_DrawHLine(x0, y1, x1);
	LCD_DrawVLine(x0, y0 + 1, y1 - 1);
	LCD_DrawVLine(x1, y0 + 1, y1 - 1);
}

void GUI_DrawRect(int x0, int y0, int x1, int y1) {
#if (GUI_WINSUPPORT)
	GUI_RECT r;
#endif

#if (GUI_WINSUPPORT)
	WM_ADDORG(x0, y0);
	WM_ADDORG(x1, y1);
	r.x0 = x0;
	r.x1 = x1;
	r.y0 = y0;
	r.y1 = y1;
	WM_ITERATE_START(&r); {
#endif
		_DrawRect(x0, y0, x1, y1);
#if (GUI_WINSUPPORT)
	} WM_ITERATE_END();
#endif

}