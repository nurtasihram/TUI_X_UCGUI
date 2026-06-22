

#include "GUI_Protected.h"
#include "GUIDebug.h"
#include "string.h"

void GUI_DispCEOL(void) {
  int y = GUI_Context.DispPosY - GUI_GetYAdjust();
  GUI_ClearRect(GUI_Context.DispPosX, y, 4000,             /* Max pos x */
                y + GUI_Context.pAFont->YDist - 1);
}


