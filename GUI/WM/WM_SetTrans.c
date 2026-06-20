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
File        : WM_SetTrans.C
Purpose     : Windows manager, optional routines
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if WM_SUPPORT_TRANSPARENCY   /* If 0, WM will not generate any code */

#include "GUIDebug.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void WM_SetHasTrans(WM_HWIN hWin) {
  WM_Obj *pWin;

  if (hWin) {
    pWin = WM_H2P(hWin);  
    /* First check if this is necessary at all */
    if ((pWin->Status & WM_SF_HASTRANS) == 0) {
      pWin->Status |= WM_SF_HASTRANS; /* Set Transparency flag */
      WM__TransWindowCnt++;          /* Increment counter for transparency windows */
      WM_InvalidateWindow(hWin);      /* Mark content as invalid */
    }
  }

}


void WM_ClrHasTrans(WM_HWIN hWin) {
  WM_Obj *pWin;

  if (hWin) {
    pWin = WM_H2P(hWin);  
    /* First check if this is necessary at all */
    if (pWin->Status & WM_SF_HASTRANS) {
      pWin->Status &= ~WM_SF_HASTRANS;
      WM__TransWindowCnt--;            /* Decrement counter for transparency windows */
      WM_InvalidateWindow(hWin);        /* Mark content as invalid */
    }
  }

}


int WM_GetHasTrans(WM_HWIN hWin) {
  int r = 0;
  WM_Obj *pWin;

  if (hWin) {
    pWin = WM_H2P(hWin);  
    r = pWin->Status & WM_SF_HASTRANS;
  }

  return r;
}

#endif /*WM_SUPPORT_TRANSPARENCY*/

/*************************** End of file ****************************/
