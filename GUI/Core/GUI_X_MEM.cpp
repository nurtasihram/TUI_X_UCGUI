#include <stdlib.h>
#include <string.h>

#include "GUI_Protected.h"
#include "GUIDebug.h"

void *GUI_ALLOC_AllocNoInit(size_t Size) {
	return malloc(Size);
}
void GUI_ALLOC_Free(void *ptr) {
	free(ptr);
}
void GUI_ALLOC_FreePtr(void **pptr) {
	GUI_ALLOC_Free(*pptr);
	*pptr = 0;
}
void *GUI_ALLOC_Realloc(void *ptr, size_t NewSize) {
	return realloc(ptr, NewSize);
}

size_t GUI_ALLOC_GetMaxSize(void) {
	return 1024 * 1024;
}

void *GUI_ALLOC_AllocInit(const void *pInitData, size_t Size) {
	void *pMem;
	if (Size == 0)
		return (void *)0;
	pMem = GUI_ALLOC_AllocNoInit(Size);
	if (pMem) {
		if (pInitData)
			memcpy(pMem, pInitData, Size);
		else
			memset(pMem, 0, Size);
	}
	return pMem;
}

void *GUI_ALLOC_AllocZero(size_t Size) {
	void *pMem;
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
