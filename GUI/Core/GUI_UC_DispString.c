

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"

static int _GetLineDistX(const uint16_t  *s, int MaxNumChars) {
  int Dist =0;
  if (s) {
    uint16_t Char;
    while (((Char = *s) != 0) && MaxNumChars >= 0) {
      s++;
      MaxNumChars--;
      Dist += GUI_GetCharDistX(Char);
    }
  }
  return Dist;
}

static int _GetLineLen(const uint16_t  *s, int MaxLen) {
  int Len =0;
  if (!s)
    return 0;
  {
    while ((*s !=0) && Len < MaxLen) {
      Len++; s++;
    }
  }
  return Len;
}

static void _DispLine_UC(const uint16_t  *s, int Len, const GUI_RECT *pRect) {
  if (GUI_Context.pClipRect_HL) {
    if (GUI_RectsIntersect(GUI_Context.pClipRect_HL, pRect) == 0)
      return;
  }
  {
    uint16_t c0;
    while (--Len >=0) {
      c0=*s++;
      GL_DispChar(c0);
    }
  }
}

static void _DispLine(const uint16_t  *s, int Len, const GUI_RECT* pr) {
  GUI_RECT r;
  r = *pr;
#if GUI_WINSUPPORT
  WM_ADDORG(r.x0,r.y0);
  WM_ADDORG(r.x1,r.y1);
  WM_ITERATE_START(&r) {
#endif
     GUI_Context.DispPosX = r.x0;
     GUI_Context.DispPosY = r.y0;
     _DispLine_UC(s, Len, &r);    /* Do the actual drawing via routine call. */
#if GUI_WINSUPPORT
  } WM_ITERATE_END();
  WM_SUBORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
#endif
}

void GUI_UC_DispString(const uint16_t  *s) {
  int xAdjust, yAdjust, xOrg;
  int FontSizeY;
  if (!s)
    return;

  FontSizeY = GUI_Context.pAFont->YSize;
  xOrg = GUI_Context.DispPosX;
 /* Adjust vertical position */
  yAdjust = GUI_GetYAdjust();
  GUI_Context.DispPosY -= yAdjust;
  for (; *s; s++) {
    GUI_RECT r;
    int LineLen= _GetLineLen(s,0x7fff);
    int xLineSize = _GetLineDistX(s, LineLen);
  /* Check if x-position needs to be changed due to h-alignment */
    switch (GUI_Context.TextAlign & GUI_TA_HORIZONTAL) {
    case GUI_TA_CENTER: xAdjust= xLineSize/2; break;
    case GUI_TA_RIGHT:  xAdjust= xLineSize; break;
    default:            xAdjust= 0;
    }
    r.x0 = GUI_Context.DispPosX -= xAdjust;
    r.x1 = r.x0 + xLineSize-1;
    r.y0 = GUI_Context.DispPosY;
    r.y1 = r.y0 + FontSizeY-1;
    _DispLine(s, LineLen, &r);
    GUI_Context.DispPosY = r.y0;
    s += LineLen;
    if (*s=='\n') {
      switch (GUI_Context.TextAlign & GUI_TA_HORIZONTAL) {
      case GUI_TA_CENTER:
      case GUI_TA_RIGHT:
        GUI_Context.DispPosX = xOrg;
        break;
      default:
        GUI_Context.DispPosX = GUI_Context.LBorder;
        break;
      }
      GUI_Context.DispPosY += GUI_GetFontDistY();
    } else {
      GUI_Context.DispPosX = r.x0+xLineSize;
    }
    if (*s==0)    /* end of string (last line) reached ? */
      break;
  }
  GUI_Context.DispPosY += yAdjust;
  GUI_Context.TextAlign &= ~GUI_TA_HORIZONTAL;

}


