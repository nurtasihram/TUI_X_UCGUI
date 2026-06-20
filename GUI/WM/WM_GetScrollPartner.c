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
File        : WM_GetScrollPartner.c
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

WM_HWIN WM_GetScrollPartner(WM_HWIN hScroll) {
  int Id = WM_GetId(hScroll);
  if (Id == GUI_ID_HSCROLL) {
    Id = GUI_ID_VSCROLL;
  } else if (Id == GUI_ID_VSCROLL) {
    Id = GUI_ID_HSCROLL;
  }
  return WM_GetDialogItem(WM_GetParent(hScroll), Id);

}

/*************************** End of file ****************************/
