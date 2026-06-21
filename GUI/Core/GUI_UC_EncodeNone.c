

#include "GUI_Protected.h"

/*********************************************************************
*
*       _GetCharCode
*
* Purpose:
*   Return the UNICODE character code of the current character.
*/
static U16 _GetCharCode(const char GUI_UNI_PTR * s) {
  return *(const U8 GUI_UNI_PTR *)s;
}

/*********************************************************************
*
*       _GetCharSize
*
* Purpose:
*   Return the number of bytes of the current character.
*/
static int _GetCharSize(const char GUI_UNI_PTR * s) {
  GUI_USE_PARA(s);
  return 1;
}

/*********************************************************************
*
*       _CalcSizeOfChar
*
* Purpose:
*   Return the number of bytes needed for the given character.
*/
static int _CalcSizeOfChar(U16 Char) {
  GUI_USE_PARA(Char);
  return 1;
}

/*********************************************************************
*
*       _Encode
*
* Purpose:
*   Encode character into 1/2/3 bytes.
*/
static int _Encode(char *s, U16 Char) {
  *s = (U8)(Char);
  return 1;
}

const GUI_UC_ENC_APILIST GUI__API_TableNone = {
  _GetCharCode,     /*  return character code as U16 */
  _GetCharSize,     /*  return size of character: 1 */
  _CalcSizeOfChar,  /*  return size of character: 1 */
  _Encode           /*  Encode character */
};

void GUI_UC_SetEncodeNone(void) {

  GUI_Context.pUC_API = &GUI__API_TableNone;

}

/*************************** End of file ****************************/
