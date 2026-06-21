

#include "GUI_Protected.h"

#if (GUI_WINSUPPORT)
static void CL_DispChar(U16 c) {
  GUI_RECT r;
  WM_ADDORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
  r.x1 = (r.x0 = GUI_Context.DispPosX) + GUI_GetCharDistX(c) - 1;
  r.y1 = (r.y0 = GUI_Context.DispPosY) + GUI_GetFontSizeY()  - 1;
  WM_ITERATE_START(&r) {
    GL_DispChar(c);
  } WM_ITERATE_END();
  if (c != '\n') {
    GUI_Context.DispPosX = r.x1 + 1;
  }
  WM_SUBORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
}
#endif

void GUI_DispChar(U16 c) {

  #if (GUI_WINSUPPORT)
    CL_DispChar(c);
  #else
    GL_DispChar(c);
  #endif

}

void GUI_DispCharAt(U16 c, I16P x, I16P y) {

  GUI_Context.DispPosX = x;
  GUI_Context.DispPosY = y;
  #if (GUI_WINSUPPORT)
    CL_DispChar(c);
  #else
    GL_DispChar(c);
  #endif

}

/*************************** End of file ****************************/
