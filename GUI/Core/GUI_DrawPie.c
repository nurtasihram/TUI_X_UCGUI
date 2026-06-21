

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"

static void _DrawPie(int x0, int y0, unsigned int r, int Angle0, int Angle1, int Type) {
  int PenSizeOld;
  PenSizeOld = GUI_GetPenSize();
  GUI_SetPenSize(r);
  r /= 2;
  GL_DrawArc(x0,y0,r,r,Angle0, Angle1);
  GUI_SetPenSize(PenSizeOld);
  GUI_USE_PARA(Type);
}

void GUI_DrawPie(int x0, int y0, int r, int a0, int a1, int Type) {

  #if GUI_WINSUPPORT
    WM_ADDORG(x0,y0);
    WM_ITERATE_START(NULL) {
  #endif
  _DrawPie( x0, y0, r, a0, a1, Type);
  #if GUI_WINSUPPORT
    } WM_ITERATE_END();
  #endif

}

/*************************** End of file ****************************/
