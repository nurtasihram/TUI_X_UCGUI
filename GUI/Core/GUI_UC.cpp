

#include "GUI_Protected.h"

int GUI_UC_GetCharSize(const char  * s) {
  int r;

  r =  GUI_Context.pUC_API->pfGetCharSize(s);

  return r;
}

uint16_t GUI_UC_GetCharCode(const char  * s) {
  uint16_t r;

  r =  GUI_Context.pUC_API->pfGetCharCode(s);

  return r;
}

int GUI_UC_Encode(char* s, uint16_t Char) {
    int r;
    r = GUI_Context.pUC_API->pfEncode(s, Char);
    return r;
}

int GUI_UC__CalcSizeOfChar(uint16_t Char) {
  return GUI_Context.pUC_API->pfCalcSizeOfChar(Char);
}

uint16_t GUI_UC__GetCharCodeInc(const char  ** ps) {
  const char  * s;
  uint16_t r;
  s   = *ps;
  r   = GUI_UC__GetCharCode(s);
  s  += GUI_UC__GetCharSize(s);
  *ps = s;
  return r;
}

int GUI_UC__NumChars2NumBytes(const char  * s, int NumChars) {
  int CharSize, NumBytes = 0;
  while (NumChars--) {
    CharSize = GUI_UC__GetCharSize(s);
    s += CharSize;
    NumBytes += CharSize;
  }
  return NumBytes;
}

int GUI_UC__NumBytes2NumChars(const char  * s, int NumBytes) {
  int CharSize, Chars = 0, Bytes = 0;
  while (NumBytes > Bytes) {
    CharSize = GUI_UC__GetCharSize(s + Bytes);
    Bytes += CharSize;
    Chars++;
  }
  return Chars;
}
