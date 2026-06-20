/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              �C/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : LCD_SelectLCD.c
Purpose     : Implementation of said routine
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"
#if GUI_WINSUPPORT
#include "WM.h"
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void LCD_SelectLCD(void) {
  #if GUI_SUPPORT_DEVICES
    GUI_Context.pDeviceAPI   = LCD_aAPI[0];
    GUI_Context.hDevData = 0;
  #endif
  GUI_Context.pClipRect_HL = &GUI_Context.ClipRect;
  LCD_SetClipRectMax();
}

/*************************** End of file ****************************/
