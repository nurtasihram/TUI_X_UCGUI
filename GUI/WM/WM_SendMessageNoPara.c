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
File        : WM_SendMessageNoPara.c
Purpose     : Implementation of WM_SendMessageNoPara
----------------------------------------------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "WM_Intern.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void WM_SendMessageNoPara(WM_HWIN hWin, int MsgId) {
  
  WM__SendMessageNoPara(hWin, MsgId);
  
}
