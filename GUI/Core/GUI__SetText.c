

#include <stdlib.h>
#include <string.h>
#include "GUI_Protected.h"

int GUI__SetText(GUI_HMEM* phText, const char* s) {
  int r = 0;
  if (GUI__strcmp_hp(*phText, s) != 0) {            /* Make sure we have a quick out if nothing changes */
    GUI_HMEM hMem;
    hMem = GUI_ALLOC_AllocNoInit(GUI__strlen(s) + 1);
    if (hMem) {
      char* pMem;
      pMem = (char*) GUI_ALLOC_h2p(hMem);
      strcpy(pMem, s);
      GUI_ALLOC_FreePtr(phText);
      *phText = hMem;
      r = 1;
    }
  }
  return r;
}

/*************************** End of file ****************************/
