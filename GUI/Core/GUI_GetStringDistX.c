

#include "GUI_Protected.h"
#include <string.h>

int GUI_GetStringDistX(const char  * s) {
  return GUI__GetLineDistX(s, GUI__strlen(s));
}


