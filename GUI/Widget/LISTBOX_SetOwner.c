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
File        : LISTBOX_SetOwner.c
Purpose     : Implementation of LISTBOX_SetOwner
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTBOX_Private.h"


#if GUI_WINSUPPORT


/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/

/*********************************************************************
*
*       LISTBOX_SetOwner
*/
void LISTBOX_SetOwner(LISTBOX_Handle hObj, WM_HWIN hOwner) {
  if (hObj) {
    LISTBOX_Obj* pObj;
    
    pObj = LISTBOX_H2P(hObj);
    ASSERT_IS_VALID_PTR(pObj);
    pObj->hOwner = hOwner;
    LISTBOX__InvalidateInsideArea(hObj);
    
  }
}



#else                            /* Avoid problems with empty object modules */
  void LISTBOX_SetOwner_C(void) {}
#endif
