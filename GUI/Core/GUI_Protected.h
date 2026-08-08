#pragma once

#include "GUI.h"
#include "LCD_Protected.h"
#include "GUIDebug.h"

#ifndef GUI_DEFAULT_FONT
#define GUI_DEFAULT_FONT    &FontMono6x8
#endif

#ifndef GUI_DEFAULT_CURSOR
#define GUI_DEFAULT_CURSOR  &CursorArrowM
#endif

#ifndef GUI_DEFAULT_BKCOLOR
#define GUI_DEFAULT_BKCOLOR RGB_BLACK
#endif

#ifndef GUI_DEFAULT_COLOR
#define GUI_DEFAULT_COLOR   RGB_WHITE
#endif

typedef enum { GUI_WRAPMODE_NONE, GUI_WRAPMODE_WORD, GUI_WRAPMODE_CHAR } GUI_WRAPMODE;

#if GUI_SUPPORT_MEMDEV
struct GUI_MEMDEV {
	int16_t x0, y0, XSize, YSize;
	int16_t BytesPerLine;
	int16_t BitsPerPixel;
	const tLCDDEV_APIList *pAPIList;
};

void GUI_MEMDEV__CopyFromLCD(GUI_MEMDEV_Handle hMem);
void GUI_MEMDEV__GetRect(RECT *pRect);

GUI_MEMDEV_Handle GUI_MEMDEV__CreateFixed(int x0, int y0, int xsize, int ysize, int Flags
											, const tLCDDEV_APIList *pMemDevAPI);

#endif

#define GUI_UC__GetCharSize(sText)  GUI.pUC_API->pfGetCharSize(sText)
#define GUI_UC__GetCharCode(sText)  GUI.pUC_API->pfGetCharCode(sText)

int   GUI_UC__CalcSizeOfChar(uint16_t Char);
uint16_t   GUI_UC__GetCharCodeInc(const char **ps);
int   GUI_UC__NumChars2NumBytes(const char *s, int NumChars);
int   GUI_UC__NumBytes2NumChars(const char *s, int NumBytes);

int  GUI__GetLineNumChars(const char *s, int MaxNumChars);
int  GUI__GetNumChars(const char *s);
int  GUI__GetLineDistX(const char *s, int Len);
int  GUI__HandleEOLine(const char **ps);
void GUI__DispLine(const char *s, int Len, const RECT *pr);
void GUI__AddSpaceHex(uint32_t v, uint8_t Len, char **ps);
void GUI__CalcTextRect(const char *pText, const RECT *pTextRectIn, RECT *pTextRectOut, int TextAlign);

int GUI__WrapGetNumCharsDisp(const char *pText, int xSize, GUI_WRAPMODE WrapMode);
int GUI__WrapGetNumCharsToNextLine(const char *pText, int xSize, GUI_WRAPMODE WrapMode);
int GUI__WrapGetNumBytesToNextLine(const char *pText, int xSize, GUI_WRAPMODE WrapMode);

bool GUI__SetText(char **ppText, const char *s);
bool GUI__strcmp(const char *s0, const char *s1);
int GUI__strlen(const char *s);
int GUI__strcpy(char *sDest, const char *sSrc);

void GL_DispChar(uint16_t c);

/************************************************************
*
*                 Callback pointers for dynamic linkage
*
*************************************************************
Dynamic linkage pointers reduces configuration hassles.
*/

extern const GUI_LOGPALETTE GUI_CursorPal;
extern const GUI_LOGPALETTE GUI_CursorPalI;

int GUI_GetBitmapPixel(PCBITMAP pBMP, unsigned x, unsigned y);

extern const GUI_UC_ENC_APILIST GUI__API_TableNone;
