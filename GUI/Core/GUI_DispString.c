

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include "GUI_Protected.h"


void GUI_DispString(const char GUI_UNI_PTR *s) {
  int xAdjust, yAdjust, xOrg;
  int FontSizeY;
  if (!s)
    return;

  FontSizeY = GUI_GetFontDistY();
  xOrg = GUI_Context.DispPosX;
 /* Adjust vertical position */
  yAdjust = GUI_GetYAdjust();
  GUI_Context.DispPosY -= yAdjust;
  for (; *s; s++) {
    GUI_RECT r;
    int LineNumChars = GUI__GetLineNumChars(s, 0x7fff);
    int xLineSize    = GUI__GetLineDistX(s, LineNumChars);
  /* Check if x-position needs to be changed due to h-alignment */
    switch (GUI_Context.TextAlign & GUI_TA_HORIZONTAL) {
      case GUI_TA_CENTER: xAdjust = xLineSize / 2; break;
      case GUI_TA_RIGHT:  xAdjust = xLineSize; break;
      default:            xAdjust = 0;
    }
    r.x0 = GUI_Context.DispPosX -= xAdjust;
    r.x1 = r.x0 + xLineSize - 1;
    r.y0 = GUI_Context.DispPosY;
    r.y1 = r.y0 + FontSizeY - 1;
    GUI__DispLine(s, LineNumChars, &r);
    GUI_Context.DispPosY = r.y0;
    s += GUI_UC__NumChars2NumBytes(s, LineNumChars);
    if ((*s == '\n') || (*s == '\r')) {
      switch (GUI_Context.TextAlign & GUI_TA_HORIZONTAL) {
      case GUI_TA_CENTER:
      case GUI_TA_RIGHT:
        GUI_Context.DispPosX = xOrg;
        break;
      default:
        GUI_Context.DispPosX = GUI_Context.LBorder;
        break;
      }
      if (*s == '\n')
        GUI_Context.DispPosY += FontSizeY;
    } else {
      GUI_Context.DispPosX = r.x0 + xLineSize;
    }
    if (*s == 0)    /* end of string (last line) reached ? */
      break;
  }
  GUI_Context.DispPosY += yAdjust;
  GUI_Context.TextAlign &= ~GUI_TA_HORIZONTAL;

}

/*************************** End of file ****************************/
