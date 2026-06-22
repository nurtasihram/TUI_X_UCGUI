

#include "GUI_Protected.h"

void GUI_DrawVLine(int x0, int y0, int y1) {
#if (GUI_WINSUPPORT)
    GUI_RECT r;
#endif

#if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    WM_ADDORGY(y1);
    r.x1 = r.x0 = x0;
    r.y0 = y0;
    r.y1 = y1;
    WM_ITERATE_START(&r); {
#endif
  LCD_DrawVLine(x0, y0, y1);
#if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
#endif

}


