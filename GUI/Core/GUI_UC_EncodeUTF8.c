

#include "GUI_Protected.h"

/*********************************************************************
*
*       _GetCharCode
*
* Purpose:
*   Return the UNICODE character code of the current character.
*/
static uint16_t _GetCharCode(const char GUI_UNI_PTR * s) {
  uint16_t r;
  uint8_t Char = *(const uint8_t*)s;
  if ((Char & 0x80) == 0) {                /* Single byte (ASCII)  */
    r = Char;
  } else if ((Char & 0xe0) == 0xc0) {      /* Double byte sequence */
    r = (Char & 0x1f) << 6;
    Char = *(++s);
    Char &= 0x3f;
    r |= Char;
  } else if ((Char & 0xf0) == 0xe0) {      /* 3 byte sequence      */
    r = (Char & 0x0f) << 12;
    Char = *(++s);
    Char &= 0x3f;
    r |= (Char << 6);
    Char = *(++s);
    Char &= 0x3f;
    r |= Char;
  } else {
    r = 1;       /* Illegal character. To avoid endless loops in upper layers, we return 1 rather than 0. */
  }
  return r;
}

/*********************************************************************
*
*       _GetCharSize
*
* Purpose:
*   Return the number of bytes of the current character.
*/
static int _GetCharSize(const char GUI_UNI_PTR * s) {
  uint8_t Char = *s;
  if ((Char & 0x80) == 0) {
    return 1;
  } else if ((Char & 0xe0) == 0xc0) {
    return 2;
  } else if ((Char & 0xf0) == 0xe0) {
    return 3;
  }
  return 1;       /* Illegal character. To avoid endless loops in upper layers, we return 1 rather than 0. */
}

/*********************************************************************
*
*       _CalcSizeOfChar
*
* Purpose:
*   Return the number of bytes needed for the given character.
*/
static int _CalcSizeOfChar(uint16_t Char) {
  int r;
  if (Char & 0xF800) {                /* Single byte (ASCII)  */
    r = 3;
  } else if (Char & 0xFF80) {         /* Double byte sequence */
    r = 2;
  } else {                            /* 3 byte sequence      */
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       _Encode
*
* Purpose:
*   Encode character into 1/2/3 bytes.
*/
static int _Encode(char *s, uint16_t Char) {
  int r;
  r = _CalcSizeOfChar(Char);
  switch (r) {
  case 1:
    *s = (char)Char;
    break;
  case 2:
    *s++ = 0xC0 | (Char >> 6);
    *s   = 0x80 | (Char & 0x3F);
    break;
  case 3:
    *s++ = 0xE0 | (Char >> 12);
    *s++ = 0x80 | ((Char >> 6) & 0x3F);
    *s   = 0x80 | (Char & 0x3F);
    break;
  }
  return r;
}

static const GUI_UC_ENC_APILIST _API_Table = {
  _GetCharCode,     /*  return character code as uint16_t (Unicode) */
  _GetCharSize,     /*  return size of character: 1/2/3        */
  _CalcSizeOfChar,  /*  return size of character: 1/2/3        */
  _Encode           /*  Encode character into 1/2/3 bytes      */
};

void GUI_UC_SetEncodeUTF8(void) {

  GUI_Context.pUC_API = &_API_Table;

}

/*************************** End of file ****************************/
