#include <stdlib.h>
#include <string.h>

import TUX.X;

void*GUI_MEM_Alloc(size_t Size) {
	return malloc(Size);
}
void GUI_MEM_Free(void *ptr) {
	free(ptr);
}
void GUI_MEM_FreePtr(void **pptr) {
	GUI_MEM_Free(*pptr);
	*pptr = 0;
}
void*GUI_MEM_Realloc(void *ptr, size_t NewSize) {
	return realloc(ptr, NewSize);
}

void GUI__memcpy(void *pDest, const void *pSrc, size_t NumBytes) {
	memcpy(pDest, pSrc, NumBytes);
}
void GUI__memmove(void *pDest, const void *pSrc, size_t NumBytes) {
	memmove(pDest, pSrc, NumBytes);
}
