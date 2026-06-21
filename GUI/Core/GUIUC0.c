

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI.h"


/*********************************************************************
*
*       GUI_UC2DB
*
* Purpose:
*   Convert Convert UNICODE 16-bit value into double byte version
*   - Byte0: First byte, msb
*   - Byte1: Last  byte, lsb
*/
void GUI_UC2DB(uint16_t Code, uint8_t* pOut) {
/* move regular ASCII to (unused) 0xE000-0xE0ff area */
  if (Code < 0x100) {
    if (Code !=0)  /* end marker ? */
      Code += 0xe000;
  } else {
/* If Low byte 0: Move to (unused) 0xE100-0xE1ff area */
    if ((Code&0xff) == 0) {
      Code = (Code>>8) +0xe100;
    }
  }
/* Convert into double byte, putting msb first*/
  *pOut++ = Code >> 8;     /* Save msb first */
  *pOut   = Code & 0xff;
}

/*********************************************************************
*
*       GUI_DB2UC
*
* Purpose:
*   Convert Unicode double byte into UNICODE.
*   - Byte0: First byte, msb
*   - Byte1: Last  byte, lsb
*/
uint16_t GUI_DB2UC(uint8_t Byte0, uint8_t Byte1) {
  if (Byte0==0)
    return 0;
  if ((Byte0&0xfe) == 0xe0) {
    if (Byte0 == 0xe0)        /* ASCII */
      return Byte1;
    return ((uint16_t)Byte1)<<8;   /* low byte was zero */
  }
  return Byte1 | (((uint16_t)Byte0)<<8);
}

/*************************** End of file ****************************/
