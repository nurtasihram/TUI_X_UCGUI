

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"
#include "GUIDebug.h"
#include "WM.h"

void GUI_GetClientRect(GUI_RECT* pRect) {
  if (!pRect)
    return;
  WM_GetClientRect(pRect);
}


