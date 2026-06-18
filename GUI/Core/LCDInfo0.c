#include <stddef.h>           /* needed for definition of NULL */
#include "LCD.h"		          /* interface definitions */
#include "LCD_Private.h"      /* private modul definitions & config */

int LCD_GetXSize(void)        { return LCD_XSIZE; }
int LCD_GetYSize(void)        { return LCD_YSIZE; }
int LCD_GetVXSize(void)       { return LCD_VXSIZE; }
int LCD_GetVYSize(void)       { return LCD_VYSIZE; }
int LCD_GetBitsPerPixel(void) { return LCD_BITSPERPIXEL; }
U32 LCD_GetNumColors(void)    { return LCD_NUM_COLORS; }
int LCD_GetYMag(void)         { return LCD_YMAG; }
int LCD_GetXMag(void)         { return LCD_XMAG; }
int LCD_GetFixedPalette(void) { return LCD_FIXEDPALETTE; }
