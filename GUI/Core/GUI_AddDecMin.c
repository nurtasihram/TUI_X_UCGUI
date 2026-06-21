

#include "GUI_Protected.h"
#include "GUIDebug.h"
#include "string.h"

void GUI_AddDecMin(I32 v, char**ps) {
  U8 Len = GUI_Long2Len(v);
  GUI_AddDecShift(v, Len, 0, ps);
}

/*************************** End of file ****************************/
