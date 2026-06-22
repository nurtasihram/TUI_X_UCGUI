

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Protected.h"
#include "GUIDebug.h"

/*********************************************************************
*
*       GUI_SetLBorder
*
* Purpose:
*   Sets the left border (for carriage return).
*/
int GUI_SetLBorder(int x) {
  int r;

  r = GUI_Context.LBorder;
  GUI_Context.LBorder = x;

  return r;
}


