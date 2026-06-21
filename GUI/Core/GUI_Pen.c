

#include "GUI_Protected.h"

U8 GUI_GetPenSize(void) {
  U8 r;

  r = GUI_Context.PenSize;

  return r;
}

U8 GUI_GetPenShape(void) {
  U8 r;

  r = GUI_Context.PenShape;

  return r;
}

U8 GUI_SetPenSize(U8 PenSize) {
  U8 r;

    r = GUI_Context.PenSize;
    GUI_Context.PenSize = PenSize;

  return r;
}

U8 GUI_SetPenShape(U8 PenShape) {
  U8 r;

    r = GUI_Context.PenShape;
    GUI_Context.PenShape = PenShape;

  return r;
}

/*************************** End of file ****************************/
