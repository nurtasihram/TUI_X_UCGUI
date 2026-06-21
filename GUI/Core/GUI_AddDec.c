

#include "GUI_Protected.h"
#include "GUIDebug.h"
#include "string.h"

void GUI_AddDec(I32 v, U8 Len, char**ps) {
  GUI_AddDecShift(v, Len, 0, ps);
}

/*************************** End of file ****************************/
