

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
  if (!GUI_RectsIntersect(&GUI_Context.ClipRect, pRect))
    return;
  while (Len--)
    GL_DispChar(*s++);
}

static void _DispLine(const uint16_t  *s, int Len, const GUI_RECT* pr) {
  GUI_RECT r;
  r = *pr;
  WM_ADDORG(r.x0,r.y0);
  WM_ADDORG(r.x1,r.y1);
  WM_ITERATE_START(&r) {
     GUI_Context.DispPos.x = r.x0;
     GUI_Context.DispPos.y = r.y0;
     _DispLine_UC(s, Len, &r);    /* Do the actual drawing via routine call. */
  } WM_ITERATE_END();
  WM_SUBORG(GUI_Context.DispPos.x, GUI_Context.DispPos.y);
}

void GUI_UC_DispString(const uint16_t  *s) {
  int xAdjust, yAdjust, xOrg;
  int FontSizeY;
  if (!s)
    return;

  FontSizeY = GUI_Context.pAFont->YSize;
  xOrg = GUI_Context.DispPos.x;
 /* Adjust vertical position */
  yAdjust = GUI_GetYAdjust();
  GUI_Context.DispPos.y -= yAdjust;
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
    r.x0 = GUI_Context.DispPos.x -= xAdjust;
    r.x1 = r.x0 + xLineSize-1;
    r.y0 = GUI_Context.DispPos.y;
    r.y1 = r.y0 + FontSizeY-1;
    _DispLine(s, LineLen, &r);
    GUI_Context.DispPos.y = r.y0;
    s += LineLen;
    if (*s=='\n') {
      switch (GUI_Context.TextAlign & GUI_TA_HORIZONTAL) {
      case GUI_TA_CENTER:
      case GUI_TA_RIGHT:
        GUI_Context.DispPos.x = xOrg;
        break;
      default:
        GUI_Context.DispPos.x = 0;
        break;
      }
      GUI_Context.DispPos.y += GUI_GetFontDistY();
    } else {
      GUI_Context.DispPos.x = r.x0+xLineSize;
    }
    if (*s==0)    /* end of string (last line) reached ? */
      break;
  }
  GUI_Context.DispPos.y += yAdjust;
  GUI_Context.TextAlign &= ~GUI_TA_HORIZONTAL;

}
