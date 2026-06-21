

#include <stddef.h>           /* needed for definition of NULL */
#include <string.h>           /* for memcpy, memset */

#include "GUI_Protected.h"
#include "GUIDebug.h"

/*********************************************************************
*
*       GUI_ALLOC_AllocInit
*
* Purpose:
*   Alloc memory block of the given size and initialize from the given pointer.
*   If the pointer is a NULL pointer, zeroinit
*/
GUI_HMEM GUI_ALLOC_AllocInit(const void *pInitData, GUI_ALLOC_DATATYPE Size) {
  GUI_HMEM hMem;

  if (Size == 0) {
    return (GUI_HMEM)0;
  }
  hMem = GUI_ALLOC_AllocNoInit(Size);
  if (hMem) {
    void *pMem;
    pMem = GUI_ALLOC_h2p(hMem);
    if (pInitData) {
      memcpy(pMem, pInitData, Size);
    } else {
      GUI_MEMSET((U8*)pMem, 0, Size);
    }
  }

  return hMem;
}

/*************************** End of file ****************************/
