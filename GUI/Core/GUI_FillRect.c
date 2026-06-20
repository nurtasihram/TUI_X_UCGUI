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
File        : GUI_FillRect.C
Purpose     : Implementation of GUI_FillRect
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Protected.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void GUI_FillRect(int x0, int y0, int x1, int y1) {
  WM_ADDORG(x0,y0);
  WM_ADDORG(x1,y1);
  {
    GUI_RECT r;
    r.x0 = x0; r.x1 = x1;
    r.y0 = y0; r.y1 = y1;
    WM_ITERATE_START(&r); {
      LCD_FillRect(x0,y0,x1,y1);
    } WM_ITERATE_END();
  }

}

/*************************** End of file ****************************/
