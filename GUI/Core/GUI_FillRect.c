

#include "GUI_Protected.h"

void GUI_FillRect(int x0, int y0, int x1, int y1) {
  WM_ADDORG(x0,y0);
  WM_ADDORG(x1,y1);
  {
    GUI_RECT r;
    r.x0 = x0; r.x1 = x1;
    r.y0 = y0; r.y1 = y1;
    WM_ITERATE_START(&r); {
      LCD_FillRect(x0,y0,x1,y1);
    } WM_ITERATE_END();
  }

}


