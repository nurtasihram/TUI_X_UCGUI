

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include <string.h>
#include "GUI_Protected.h"


static char _GotoY(int y) {
  GUI_Context.DispPosY = y;
  return 0;
}

static char _GotoX(int x) {
  GUI_Context.DispPosX = x;
  return 0;
}

char GUI_GotoY(int y) {
  char r;

  r = _GotoY(y);

  return r;
}

char GUI_GotoX(int x) {
  char r;

  r = _GotoX(x);

  return r;
}

char GUI_GotoXY(int x, int y) {
  char r;

  r  = _GotoX(x);
  r |= _GotoY(y);

  return r;
}

/*************************** End of file ****************************/
