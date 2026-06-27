#include <stdlib.h>
#include <string.h>

#include "GUI_Protected.h"
#include "GUIDebug.h"

GUI_HMEM GUI_ALLOC_AllocNoInit(size_t Size) {
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

size_t GUI_ALLOC_GetMaxSize(void) {
	return 1024 * 1024;
}

GUI_HMEM GUI_ALLOC_AllocInit(const void *pInitData, size_t Size) {
	GUI_HMEM pMem;
	if (Size == 0)
		return (GUI_HMEM)0;
	pMem = GUI_ALLOC_AllocNoInit(Size);
	if (pMem) {
		if (pInitData)
			memcpy(pMem, pInitData, Size);
		else
			memset(pMem, 0, Size);
	}
	return pMem;
}

GUI_HMEM GUI_ALLOC_AllocZero(size_t Size) {
	GUI_HMEM pMem;
	pMem = GUI_ALLOC_AllocNoInit(Size);
	if (pMem)
		memset(pMem, 0, Size);
	return pMem;
}

void GUI__memcpy(void *pDest, const void *pSrc, size_t NumBytes) {
	memcpy(pDest, pSrc, NumBytes);
}

void GUI__memmove(void *pDest, const void *pSrc, size_t NumBytes) {
	memmove(pDest, pSrc, NumBytes);
}
