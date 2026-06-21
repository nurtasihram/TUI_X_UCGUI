

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"
#include "GUIDebug.h"

/*********************************************************************
*
*       LCD_GetPixelIndex
*
* NOTE:
*   We can not use the standard clipping which we use for drawing
*   operations as it is perfectly legal to read pixels outside of
*   the clipping area. We therefor get the bounding rectangle of the
*   device and use it for clipping.
*/
unsigned LCD_GetPixelIndex(int x, int y)  {
  LCD_RECT r;
  LCDDEV_L0_GetRect(&r);
  if (x < r.x0) {
    return 0;
  }
  if (x > r.x1) {
    return 0;
  }
  if (y < r.y0) {
    return 0;
  }
  if (y > r.y1) {
    return 0;
  }
  return LCDDEV_L0_GetPixelIndex(x,y);
}

/*************************** End of file ****************************/
