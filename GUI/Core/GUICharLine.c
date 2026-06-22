

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include "GUI_Protected.h"

static void _DispLine(const char GUI_UNI_PTR *s, int MaxNumChars, const GUI_RECT *pRect) {
  /* Check if we have anything to do at all ... */
  if (GUI_Context.pClipRect_HL) {
    if (GUI_RectsIntersect(GUI_Context.pClipRect_HL, pRect) == 0)
      return;
  }
  if (GUI_Context.pAFont->pafEncode) {
    GUI_Context.pAFont->pafEncode->pfDispLine(s, MaxNumChars);
  } else {
    uint16_t Char;
    while (--MaxNumChars >= 0) {
      Char = GUI_UC__GetCharCodeInc(&s);
      GUI_Context.pAFont->pfDispChar(Char);
      if (GUI_pfDispCharStyle) {
        GUI_pfDispCharStyle(Char);
      }
    }
  }
}

int GUI__GetLineNumChars(const char GUI_UNI_PTR *s, int MaxNumChars) {
  int NumChars = 0;
  if (s) {
      if (GUI_Context.pAFont->pafEncode) {
        return GUI_Context.pAFont->pafEncode->pfGetLineLen(s, MaxNumChars);
      }
    for (; NumChars < MaxNumChars; NumChars++) {
      uint16_t Data = GUI_UC__GetCharCodeInc(&s);
      if ((Data == 0) || (Data == '\n')) {
        break;
      }
    }
  }
  return NumChars;
}

/*********************************************************************
*
*       GUI_GetLineDistX
*
*  This routine is used to calculate the length of a line in pixels.
*/
int GUI__GetLineDistX(const char GUI_UNI_PTR *s, int MaxNumChars) {
  int Dist = 0;
  if (s) {
    uint16_t Char;
      if (GUI_Context.pAFont->pafEncode) {
        return GUI_Context.pAFont->pafEncode->pfGetLineDistX(s, MaxNumChars);
      }
    while (--MaxNumChars >= 0) {
      Char  = GUI_UC__GetCharCodeInc(&s);
      Dist += GUI_GetCharDistX(Char);
    }
  }
  return Dist;
}

void GUI__DispLine(const char GUI_UNI_PTR *s, int MaxNumChars, const GUI_RECT* pr) {
  GUI_RECT r;
  {
    r = *pr;
#if GUI_WINSUPPORT
    WM_ADDORG(r.x0, r.y0);
    WM_ADDORG(r.x1, r.y1);
    WM_ITERATE_START(&r) {
#endif
      GUI_Context.DispPosX = r.x0;
      GUI_Context.DispPosY = r.y0;
      /* Do the actual drawing via routine call. */
      _DispLine(s, MaxNumChars, &r);
#if GUI_WINSUPPORT
    } WM_ITERATE_END();
    WM_SUBORG(GUI_Context.DispPosX, GUI_Context.DispPosY);
#endif
  }
}


