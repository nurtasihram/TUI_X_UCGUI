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
File        : WM_SetDesktopColor.c
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include "WM_Intern_ConfDep.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


GUI_COLOR WM_SetDesktopColor(GUI_COLOR Color) {
  GUI_COLOR r;
  r = WM__aBkColor;
  WM__aBkColor = Color;
  WM_InvalidateWindow(WM__ahDesktopWin);
  return r;
}


void WM_SetDesktopColors(GUI_COLOR Color) {
  WM_SetDesktopColor(Color);
}

#else
  void WM_SetDesktopColor_c(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
