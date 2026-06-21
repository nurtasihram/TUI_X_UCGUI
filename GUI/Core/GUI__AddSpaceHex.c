

#include "GUI.h"
#include "GUI_Protected.h"

void GUI__AddSpaceHex(U32 v, U8 Len, char** ps) {
  char* s = *ps;
  *s++ = ' ';
  *ps = s;
  GUI_AddHex(v, Len, ps);
}

/*************************** End of file ****************************/
