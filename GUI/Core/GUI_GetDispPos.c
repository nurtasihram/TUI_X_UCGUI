

#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI_GetDispPosX
*
* Purpose:
*   Return X-component of current display position
*/
int GUI_GetDispPosX(void) {
  int r;

  r = GUI_Context.DispPosX;

  return r;
}

/*********************************************************************
*
*       GUI_GetDispPosY
*
* Purpose:
*   Return Y-component of current display position
*/
int GUI_GetDispPosY(void) {
  int r;

  r = GUI_Context.DispPosY;

  return r;
}


