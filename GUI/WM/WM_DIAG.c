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
File        : WM_DIAG.c
Purpose     : Implementation of WM_DIAG_ ... functions
----------------------------------------------------------------------
*/

#include <stddef.h>
#include "WM_Intern.h"

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/

/*********************************************************************
*
*       _ShowInvalid
*
* Function:
*   Debug code: shows invalid areas
*/
static void _ShowInvalid(WM_HWIN hWin) {
  GUI_CONTEXT Context = GUI_Context;
  GUI_RECT rClient;
  WM_Obj * pWin;
  pWin = WM_H2P(hWin);
  rClient = pWin->InvalidRect;
  GUI_MoveRect(&rClient, -pWin->Rect.x0, -pWin->Rect.y0);
  WM_SelectWindow(hWin);
  GUI_SetColor(GUI_GREEN);
  GUI_SetBkColor(GUI_GREEN);
  GUI_FillRect(rClient.x0, rClient.y0, rClient.x1, rClient.y1);
  Sleep(20);
  GUI_Context = Context;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void WM_DIAG_EnableInvalidationColoring(int OnOff) {
  if (OnOff) {
    WM__pfShowInvalid = _ShowInvalid;
  } else {
    WM__pfShowInvalid = NULL;
  }
}
