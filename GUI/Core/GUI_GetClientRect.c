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
File        : GUI_GetClientRect.c
Purpose     : Implementation of GUI_GetClientRect
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"
#include "GUIDebug.h"
#include "WM.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void GUI_GetClientRect(GUI_RECT* pRect) {
  if (!pRect)
    return;
  WM_GetClientRect(pRect);
}

/*************************** End of file ****************************/
