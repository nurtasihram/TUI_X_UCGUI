

#include "LCD.h"              /* interface definitions */
#include "LCD_Protected.h"    /* inter modul definitions */

LCD_COLOR LCD_GetPixelColor(int x, int y) {
  return (LCD_GetPixelIndex(x,y));
}

/*************************** End of file ****************************/
