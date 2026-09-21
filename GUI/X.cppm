export module TUX.X;

export import TUX.Types;
export import TUX.LCD;

export {

/**** Init ****/
void GUI_X_Init(void);
LCDDEV *GUI_X_GetLCD(void);

/**** Timing routines - required for blinking ****/
int GUI_TIME_Get(void);

/**** Memory routines ****/
void*GUI_MEM_Alloc(size_t Size);
void GUI_MEM_Free(void *ptr);
void GUI_MEM_FreePtr(void **pptr);
void*GUI_MEM_Realloc(void *ptr, size_t NewSize);

void GUI__memcpy(void *pDest, const void *pSrc, size_t NumBytes);
void GUI__memmove(void *pDest, const void *pSrc, size_t NumBytes);

}
