

#include "GUI_Protected.h"

U8 GUI_SetLineStyle(U8 LineStyle) {
  U8 r;

    r = GUI_Context.LineStyle;
    GUI_Context.LineStyle = LineStyle;

  return r;
}

/*************************** End of file ****************************/
