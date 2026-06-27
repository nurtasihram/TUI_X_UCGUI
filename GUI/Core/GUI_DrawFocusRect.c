#include "GUI_Protected.h"

void GUI_DrawFocusRect(const GUI_RECT *pRect, int Dist) {
	GUI_RECT r;
	GUI__ReduceRect(&r, pRect, Dist);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r); {
		int i;
		for (i = r.x0; i <= r.x1; i += 2) {
			LCD_DrawPixel(i, r.y0);
			LCD_DrawPixel(i, r.y1);
		}
		for (i = r.y0; i <= r.y1; i += 2) {
			LCD_DrawPixel(r.x0, i);
			LCD_DrawPixel(r.x1, i);
		}
	} WM_ITERATE_END();

}

void GUI_DrawVLine(int x0, int y0, int y1) {
	GUI_RECT r;
	WM_ADDORG(x0, y0);
	WM_ADDORGY(y1);
	r.x1 = r.x0 = x0;
	r.y0 = y0;
	r.y1 = y1;
	WM_ITERATE_START(&r); {
		LCD_DrawVLine(x0, y0, y1);
	} WM_ITERATE_END();

}

void GUI_DrawHLine(int y0, int x0, int x1) {
	GUI_RECT r;
	WM_ADDORG(x0, y0);
	WM_ADDORGX(x1);
	r.x0 = x0;
	r.x1 = x1;
	r.y1 = r.y0 = y0;
	WM_ITERATE_START(&r) {
		LCD_DrawHLine(x0, y0, x1);
	} WM_ITERATE_END();
}
