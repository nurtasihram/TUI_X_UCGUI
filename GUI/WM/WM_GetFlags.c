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
File        : WM_GetFlags.c
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

/*********************************************************************
*
*         Public code
*
**********************************************************************
*/

U16 WM_GetFlags(WM_HWIN hWin) {
  U16 r = 0;
  if (hWin) {

    r = WM_H2P(hWin)->Status;

  }
  return r;
}

/*************************** End of file ****************************/
