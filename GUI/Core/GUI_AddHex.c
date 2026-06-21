

#include "GUI_Protected.h"

static const char acHex[16] =  {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void GUI_AddHex(U32 v, U8 Len, char**ps) {
  char *s = *ps;
  if (Len > 8) {
    return;
  }
  (*ps) += Len;
  **ps   = '\0';     /* Make sure string is 0-terminated */
  while(Len--) {
    *(s + Len) = acHex[v & 15];
    v >>= 4;
  }
}

/*************************** End of file ****************************/
