

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

void GUI_SelectLCD(void) {
  LCD_SelectLCD();
  LCD_UpdateColorIndices();
  WM_Activate();
}

/*************************** End of file ****************************/
