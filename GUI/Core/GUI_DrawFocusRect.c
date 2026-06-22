

#include "GUI_Protected.h"

static void _DrawFocusRect(const GUI_RECT* pr) {
  int i;
  for (i = pr->x0; i <= pr->x1; i += 2) {
    LCD_DrawPixel(i, pr->y0);
    LCD_DrawPixel(i, pr->y1);
  }
  for (i = pr->y0; i <= pr->y1; i += 2) {
    LCD_DrawPixel(pr->x0, i);
    LCD_DrawPixel(pr->x1, i);
  }
}

void GUI_DrawFocusRect(const GUI_RECT *pRect, int Dist) {
  GUI_RECT r;
  GUI__ReduceRect(&r, pRect, Dist);

#if (GUI_WINSUPPORT)
    WM_ADDORG(r.x0, r.y0);
    WM_ADDORG(r.x1, r.y1);
    WM_ITERATE_START(&r); {
#endif
    _DrawFocusRect(&r);
#if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
#endif

}


