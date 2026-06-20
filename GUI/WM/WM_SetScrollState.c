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
File        : WM_SetSize.c
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void WM_SetScrollState(WM_HWIN hWin, const WM_SCROLL_STATE* pState) {
  if (hWin && pState) {
    WM_MESSAGE Msg;
    Msg.MsgId = WM_SET_SCROLL_STATE;
    Msg.Data.p = (const void*)pState;
    WM_SendMessage(hWin, &Msg);
  }
}

/*************************** End of file ****************************/
