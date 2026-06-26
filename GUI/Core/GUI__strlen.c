

#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI__strlen
*
* Purpose:
*  Replacement for the strlen function. The advantage is that it can
*  be called with a NULL pointer, in which case -1 is returned.
*/
int GUI__strlen(const char  * s) {
  int r = -1;
  if (s) {
    do {
      r++;
    } while (*s++);
  }
  return r;
}


