

#include "GUI_Protected.h"

static int _IsLineEnd(uint16_t Char) {
  if (!Char || (Char == '\n')) {
    return 1;
  }
  return 0;
}

static int _GetWordWrap(const char  * s, int xSize) {
  int xDist = 0, NumChars = 0, WordWrap = 0;
  uint16_t Char, PrevChar = 0;
  while (1) {
    Char = GUI_UC__GetCharCodeInc(&s);   /* Similar to:  *s++ */
    /* Let's first check if the line end is reached. In this case we are done. */
    if (_IsLineEnd(Char)) {
      WordWrap = NumChars;
      break;
    }
    /* If current character is a space, we found a wrap position */
    if ((Char == ' ') && (Char != PrevChar)) {
      WordWrap = NumChars;
    }
    PrevChar = Char;
    xDist += GUI_GetCharDistX(Char);
    if ((xDist <= xSize) || (NumChars == 0)) {
      NumChars++;
    } else {
      break;
    }
  }
  if (!WordWrap) {
    WordWrap = NumChars;
  }
  return WordWrap;
}

static int _GetCharWrap(const char  * s, int xSize) {
  int xDist = 0, NumChars = 0;
  uint16_t Char;
  while ((Char = GUI_UC__GetCharCodeInc(&s)) != 0) {
    xDist += GUI_GetCharDistX(Char);
    if ((NumChars && (xDist > xSize)) || (Char == '\n')) {
      break;
    }
    NumChars++;
  }
  return NumChars;
}

static int _GetNoWrap(const char  * s) {
  return GUI__GetLineNumChars(s, 0x7FFF);
}

/*********************************************************************
*
*       GUI__WrapGetNumCharsDisp
*
* Returns:
*  Number of characters to display in the line.
*  Trailing spaces and line end character are
*  not counted
*/
int GUI__WrapGetNumCharsDisp(const char  * pText, int xSize, GUI_WRAPMODE WrapMode) {
  int r;
  switch (WrapMode) {
  case GUI_WRAPMODE_WORD:
    r = _GetWordWrap(pText, xSize);
    break;
  case GUI_WRAPMODE_CHAR:
    r = _GetCharWrap(pText, xSize);
    break;
  default:
    r = _GetNoWrap(pText);
  }
  return r;
}

int GUI__WrapGetNumCharsToNextLine(const char  * pText, int xSize, GUI_WRAPMODE WrapMode) {
  int NumChars;
  uint16_t Char;
  NumChars = GUI__WrapGetNumCharsDisp(pText, xSize, WrapMode);
  pText   += GUI_UC__NumChars2NumBytes(pText, NumChars);
  Char     = GUI_UC__GetCharCodeInc(&pText);
  if (Char == '\n') {
    NumChars++;
  } else {
    if (WrapMode == GUI_WRAPMODE_WORD) {
      while (Char == ' ') {
        NumChars++;
        Char = GUI_UC__GetCharCodeInc(&pText);
      }
    }
  }
  return NumChars;
}

int GUI__WrapGetNumBytesToNextLine(const char  * pText, int xSize, GUI_WRAPMODE WrapMode) {
  int NumChars, NumBytes;
  NumChars = GUI__WrapGetNumCharsToNextLine(pText, xSize, WrapMode);
  NumBytes = GUI_UC__NumChars2NumBytes(pText, NumChars);
  return NumBytes;
}



