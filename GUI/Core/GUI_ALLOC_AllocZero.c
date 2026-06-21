

#include <stddef.h>           /* needed for definition of NULL */
#include <string.h>           /* for memcpy, memset */

#include "GUI_Protected.h"
#include "GUIDebug.h"

GUI_HMEM GUI_ALLOC_AllocZero(GUI_ALLOC_DATATYPE Size) {
  GUI_HMEM hMem;
  hMem = GUI_ALLOC_AllocNoInit(Size);
  if (hMem) {
    GUI_MEMSET((U8*)GUI_ALLOC_h2p(hMem), 0, Size);   /* Zeroinit ! */
  }

  return hMem;
}

/*************************** End of file ****************************/
