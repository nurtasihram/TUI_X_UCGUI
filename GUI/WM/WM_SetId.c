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
File        : WM_SetId.c
Purpose     : Implementation of WM_SetId
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/

void WM_SetId(WM_HWIN hObj, int Id) {
  WM_MESSAGE Msg;
  Msg.MsgId  = WM_SET_ID;
  Msg.Data.v = Id;
  WM_SendMessage(hObj, &Msg);
}

/*************************** End of file ****************************/
