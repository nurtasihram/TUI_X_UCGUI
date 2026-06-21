#include <stdlib.h>
#include <string.h>

#include "GUI_Protected.h"
#include "GUIDebug.h"

GUI_HMEM GUI_ALLOC_AllocNoInit(GUI_ALLOC_DATATYPE Size) {
	return malloc(Size);
}
void GUI_ALLOC_Free(GUI_HMEM hMem) {
	free(hMem);
}
void GUI_ALLOC_FreePtr(GUI_HMEM *ph) {
	GUI_ALLOC_Free(*ph);
	*ph = 0;
}
GUI_HMEM GUI_ALLOC_Realloc(GUI_HMEM hOld, int NewSize) {
	return realloc(hOld, NewSize);
}

void *GUI_ALLOC_h2p(GUI_HMEM  hMem) {
	return hMem;
}

GUI_ALLOC_DATATYPE GUI_ALLOC_GetMaxSize(void) {
	return 1024 * 1024;
}

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
		}
		else {
			GUI_MEMSET((uint8_t *)pMem, 0, Size);
		}
	}

	return hMem;
}

GUI_HMEM GUI_ALLOC_AllocZero(GUI_ALLOC_DATATYPE Size) {
	GUI_HMEM hMem;
	hMem = GUI_ALLOC_AllocNoInit(Size);
	if (hMem) {
		GUI_MEMSET((uint8_t *)GUI_ALLOC_h2p(hMem), 0, Size);   /* Zeroinit ! */
	}

	return hMem;
}
