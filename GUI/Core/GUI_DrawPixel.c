

#include "GUI_Protected.h"

void GUI_DrawPixel(int x, int y) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif

  #if (GUI_WINSUPPORT)
    WM_ADDORG(x,y);
    r.x0 = r.x1 = x;
    r.y0 = r.y1 = y;
    WM_ITERATE_START(&r); {
  #endif
  LCD_HL_DrawPixel(x,y);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif

}

/*************************** End of file ****************************/
