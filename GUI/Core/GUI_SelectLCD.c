

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

void GUI_SelectLCD(void) {

  #if GUI_SUPPORT_DEVICES
    LCD_SelectLCD();
    LCD_UpdateColorIndices();
    #if GUI_WINSUPPORT
      WM_Activate();
    #endif
  #endif

}

/*************************** End of file ****************************/
