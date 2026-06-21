

#include "GUI_Protected.h"
#include "GUIDebug.h"

void GUI_AddBin(U32 v, U8 Len, char** ps) {
  char *s = *ps;
#if GUI_DEBUG_LEVEL >1
  if (Len > 32) {
    GUI_DEBUG_WARN("Can not display more than 32 bin. digits");
    Len = 32;
  }
#endif
  (*ps) += Len;
  **ps   = '\0';     /* Make sure string is 0-terminated */
  while(Len--) {
    *(s + Len) = (char)('0' + (v & 1));
    v >>= 1;
  }
}

/*************************** End of file ****************************/
