#include "GUI_Protected.h"
#include "GUIDebug.h"

void GUI_SetDefault(void) {
	GUI_SetBkColor(GUI_DEFAULT_BKCOLOR);
	GUI_SetColor(GUI_DEFAULT_COLOR);
	GUI_SetPenSize(1);
	GUI_SetTextAlign(0);
	GUI_SetTextMode(0);
	GUI_SetDrawMode(0);
	GUI_SetFont(GUI_DEFAULT_FONT);
	GUI_SetLineStyle(GUI_LS_SOLID);
}

static void _DrawRect(int x0, int y0, int x1, int y1) {
	LCD_DrawHLine(x0, y0, x1);
	LCD_DrawHLine(x0, y1, x1);
	LCD_DrawVLine(x0, y0 + 1, y1 - 1);
	LCD_DrawVLine(x1, y0 + 1, y1 - 1);
}

void GUI_DrawRect(int x0, int y0, int x1, int y1) {
#if (GUI_WINSUPPORT)
	int Off;
	GUI_RECT r;
#endif

#if (GUI_WINSUPPORT)
	Off = GUI_Context.PenSize - 1;
	WM_ADDORG(x0, y0);
	WM_ADDORG(x1, y1);
	r.x0 = x0 - Off;
	r.x1 = x1 + Off;
	r.y0 = y0 - Off;
	r.y1 = y1 + Off;
	WM_ITERATE_START(&r); {
#endif
		_DrawRect(x0, y0, x1, y1);
#if (GUI_WINSUPPORT)
	} WM_ITERATE_END();
#endif

}