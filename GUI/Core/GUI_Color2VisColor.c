

#include "GUI_Protected.h"

GUI_COLOR GUI_Color2VisColor(GUI_COLOR color) {
  I16 Index;
  GUI_COLOR r;

  Index = (color);
  r = (Index);

  return r;
}

char GUI_ColorIsAvailable(GUI_COLOR color) {
  return (GUI_Color2VisColor(color) == color) ? 1 : 0;
}

U32 GUI_CalcVisColorError(GUI_COLOR color) {
  return GUI_CalcColorDist(color, GUI_Color2VisColor(color));
}

/*************************** End of file ****************************/
