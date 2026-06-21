

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"

static uint16_t DB2SJIS(uint8_t Byte0, uint8_t Byte1) {
  return Byte1 | (((uint16_t)Byte0)<<8);
}

static int _GetLineDistX_SJIS(const char GUI_UNI_PTR *s, int Len) {
  int Dist =0;
  if (s) {
    uint8_t c0;
    while (((c0=*(const uint8_t*)s) !=0) && Len >=0) {
      s++; Len--;
      if (c0 > 127) {
        uint8_t  c1 = *(const uint8_t*)s++;
        Len--;
        Dist += GUI_GetCharDistX(DB2SJIS(c0, c1));
      } else {
        Dist += GUI_GetCharDistX(c0);
      }
    }
  }
  return Dist;
}

/*********************************************************************
*
*       _GetLineLen_SJIS
* Purpose:
*   Returns the number of characters in a string.
*
* NOTE:
*   The return value can be used as offset into the
*   string, which means that double characters count double
*/
static int _GetLineLen_SJIS(const char GUI_UNI_PTR *s, int MaxLen) {
  int Len =0;
  uint8_t c0;
  while (((c0=*(const uint8_t*)s) !=0) && Len < MaxLen) {
    s++;
    if (c0 > 127) {
      Len++; s++;
    } else {
      switch (c0) {
      case '\n': return Len;
      }
    }
    Len++;
  }
  return Len;
}

static void _DispLine_SJIS(const char GUI_UNI_PTR *s, int Len) {
  uint8_t c0;
  while (--Len >=0) {
    c0=*(const uint8_t*)s++;
    if (c0 > 127) {
      uint8_t  c1 = *(const uint8_t*)s++;
      Len--;
      GL_DispChar (DB2SJIS(c0, c1));
    } else {
      GL_DispChar(c0);
    }
  }
}

const tGUI_ENC_APIList GUI_ENC_APIList_SJIS = {
  _GetLineDistX_SJIS,
  _GetLineLen_SJIS,
  _DispLine_SJIS
};

/*************************** End of file ****************************/
