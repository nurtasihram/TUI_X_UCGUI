#pragma once

#include "LCDConf.h"            /* Configuration header file */

/*******************************************************************
*
*               Defaults for config switches
*
********************************************************************s
*/

#ifndef LCD_NUM_COLORS
  #define LCD_NUM_COLORS (1L<<LCD_BITSPERPIXEL)
#endif

/* Allow use of multiple physical lines for one logical line ...
   Used for delta-type LCDs, where value should be 2 */

#ifndef LCD_SWAP_BYTE_ORDER
  #define LCD_SWAP_BYTE_ORDER (0)    /* only for 16 bit bus, default is not swapped */
#endif

/* Optionally swap red and blue components */
#ifndef LCD_SWAP_RB
  #define LCD_SWAP_RB 0
#endif

#if !defined (LCD_CONTROLLER) && defined (LCD_CONTROLLER_0)
  #define LCD_CONTROLLER LCD_CONTROLLER_0
#endif

#if !defined (LCD_XSIZE) && defined (LCD_XSIZE_0)
  #define LCD_XSIZE          LCD_XSIZE_0
#endif

#if !defined (LCD_YSIZE) && defined (LCD_YSIZE_0)
  #define LCD_YSIZE          LCD_YSIZE_0
#endif

#if !defined (LCD_BITSPERPIXEL) && defined (LCD_BITSPERPIXEL_0)
  #define LCD_BITSPERPIXEL   LCD_BITSPERPIXEL_0
#endif

#if !defined (LCD_VYSIZE) && defined (LCD_VYSIZE_0)
  #define LCD_VYSIZE          LCD_VYSIZE_0
#endif

#if !defined (LCD_PHYSCOLORS)
  #if   (LCD_BITSPERPIXEL == 1) | (LCD_BITSPERPIXEL == 2) | (LCD_BITSPERPIXEL == 4)
    #ifndef LCD_FIXEDPALETTE
      #define LCD_FIXEDPALETTE LCD_BITSPERPIXEL
    #endif
  #endif
#endif /* defined (LCD_PHYSCOLORS) */

#if !defined (LCD_FIXEDPALETTE)
  #if !defined (LCD_PHYSCOLORS)
    #if LCD_BITSPERPIXEL == 8
    //  #define LCD_FIXEDPALETTE 8666
      #define LCD_FIXEDPALETTE 233

    #elif LCD_BITSPERPIXEL == 12
      #define LCD_FIXEDPALETTE 44412
    #elif LCD_BITSPERPIXEL == 15
      #define LCD_FIXEDPALETTE 555
    #elif LCD_BITSPERPIXEL == 16
      #define LCD_FIXEDPALETTE 565
    #else
      #define LCD_FIXEDPALETTE 0
    #endif
  #else
    #define LCD_FIXEDPALETTE 0
  #endif
#endif

/* In order to avoid warnings for undefined parameters */
#ifndef LCD_USE_PARA
  #define LCD_USE_PARA(para) para=para;
#endif

#define LCD_PIXELINDEX U32
