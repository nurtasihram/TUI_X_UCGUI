

#include <stddef.h>           /* needed for definition of NULL */
#include "LCD.h"
#include "GUI_Private.h"
#include "LCD_Private.h"

/*********************************************************************
*
*       LCD_MixColors256
*
* Purpose:
*   Mix 2 colors.
*
* Parameters:
*   Intens:    Intensity of first color in 257 steps, from 0 to 256, where 256 equals 100%
*/
RGB_COLOR LCD_MixColors256(RGB_COLOR Color, RGB_COLOR BkColor, unsigned Intens) {
  /* Calc Color seperations for FgColor first */
  uint32_t R = (Color & 0xff)    * Intens;
  uint32_t G = (Color & 0xff00)  * Intens;
  uint32_t B = (Color & 0xff0000)* Intens;
  /* Add Color seperations for BkColor */
  Intens = 256 - Intens;
  R += (BkColor & 0xff)     * Intens;
  G += (BkColor & 0xff00)   * Intens;
  B += (BkColor & 0xff0000) * Intens;
  R = (R >> 8);
  G = (G >> 8) & 0xff00;
  B = (B >> 8) & 0xff0000;
  return R + G + B;
}


