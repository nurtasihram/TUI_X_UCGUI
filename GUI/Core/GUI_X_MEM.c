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
			GUI_MEMSET((uint8_t *)pMem, 0, Size);
	}
	return pMem;
}

GUI_HMEM GUI_ALLOC_AllocZero(size_t Size) {
	GUI_HMEM pMem;
	pMem = GUI_ALLOC_AllocNoInit(Size);
	if (pMem)
		GUI_MEMSET((uint8_t *)pMem, 0, Size);   /* Zeroinit ! */
	return pMem;
}

/*********************************************************************
*
*       GUI__memset
*
* Purpose:
*  Equivalent to the memset function, for 16 bit Data.
*/
void GUI__memset16(uint16_t *p, uint16_t Fill, int NumWords) {
	/* Code for 16 bit CPUs */
	if (sizeof(int) == 2) { /* May some compilers generate a warning at this line: Condition is alwaws true/false */
		if (NumWords >= 8) {  /* May some compilers generate a warning at this line: Unreachable code */
			*p++ = Fill;
			*p++ = Fill;
			*p++ = Fill;
			*p++ = Fill;
			*p++ = Fill;
			*p++ = Fill;
			*p++ = Fill;
			*p++ = Fill;
			NumWords -= 8;
		}
		while (NumWords) {
			*p++ = Fill;
			NumWords--;
		}
	}
	else {
		int NumInts;          /* May some compilers generate a warning at this line: Unreachable code */
		/* Write 16 bit until we are done or have reached an int boundary */
		if (2 & (uint32_t)p) {
			*p++ = Fill;
			NumWords--;
		}
		/* Write Ints */
		NumInts = (unsigned)NumWords / (sizeof(int) / 2);
		if (NumInts) {
			int FillInt;
			int *pInt;
			FillInt = Fill * 0x10001;

			pInt = (int *)p;
			/* Fill large amount of data at a time */
			if (NumInts >= 4) {
				do {
					*pInt = FillInt;
					*(pInt + 1) = FillInt;
					*(pInt + 2) = FillInt;
					*(pInt + 3) = FillInt;
					pInt += 4;
				} while ((NumInts -= 4) >= 4);
			}
			/* Fill one int at a time */
			while (NumInts) {
				*pInt++ = FillInt;
				NumInts--;
			}
			p = (uint16_t *)pInt;
		}
		/* Fill the remainder */
		if (NumWords & 1) {
			*p = Fill;
		}
	}
}

/*********************************************************************
*
*       GUI__memset
*
* Purpose:
*  Replacement for the memset function. The advantage is high speed
*  on all systems (sometime up to 10 times as fast as the one
*  in the library)
*  Main idea is to write int-wise.
*/
void GUI__memset(uint8_t *p, uint8_t Fill, int NumBytes) {
	int NumInts;
	/* Write bytes until we are done or have reached an int boundary */
	while (NumBytes && ((sizeof(int) - 1) & (uint32_t)p)) {
		*p++ = Fill;
		NumBytes--;
	}
	/* Write Ints */
	NumInts = (unsigned)NumBytes / sizeof(int);
	if (NumInts) {
		int FillInt;
		int *pInt;
		NumBytes &= (sizeof(int) - 1);
		if (sizeof(int) == 2) {        /* May some compilers generate a warning at this line: Condition is alwaws true/false */
			FillInt = Fill * 0x101;      /* May some compilers generate a warning at this line: Unreachable code */
		}
		else if (sizeof(int) == 4) { /* May some compilers generate a warning at this line: Condition is alwaws true/false */
			FillInt = Fill * 0x1010101;  /* May some compilers generate a warning at this line: Unreachable code */
		}
		pInt = (int *)p;
		/* Fill large amount of data at a time */
		while (NumInts >= 4) {
			*pInt++ = FillInt;
			*pInt++ = FillInt;
			*pInt++ = FillInt;
			*pInt++ = FillInt;
			NumInts -= 4;
		}
		/* Fill one int at a time */
		while (NumInts) {
			*pInt++ = FillInt;
			NumInts--;
		}
		p = (uint8_t *)pInt;
	}
	/* Fill the remainder byte wise */
	while (NumBytes) {
		*p++ = Fill;
		NumBytes--;
	}
}
