#pragma once

#include <stdlib.h>

#include "LCD.h"
#include "GUIConf.h"

typedef const char *GUI_ConstString;

typedef struct {
	void(*pfDraw)(int x0, int y0, int xsize, int ysize, const uint8_t  *pPixel, const GUI_LOGPALETTE  *pLogPal, int xMag, int yMag);
} GUI_BITMAP_METHODS;

struct GUI_BITMAP {
	uint16_t XSize;
	uint16_t YSize;
	uint16_t BytesPerLine;
	uint16_t BitsPerPixel;
	const uint8_t  *pData;
	const GUI_LOGPALETTE  *pPal;
};
using CBITMAP = const GUI_BITMAP;
using PCBITMAP = const GUI_BITMAP *;

struct GUI_PID_STATE {
	int16_t x, y;
	uint8_t Pressed;
};

/*
	  ****************************************
	  *                                      *
	  *      FONT structures (new in V1.10)  *
	  *                                      *
	  ****************************************
*/

struct GUI_CHARINFO {
	uint8_t XSize;
	uint8_t XDist;
	uint8_t BytesPerLine;
	const uint8_t  *pData;
};
struct FONT_PROP {
	uint16_t First, Last;
	const GUI_CHARINFO *paCharInfo; /* address of first character    */
	const FONT_PROP *pNext; /* pointer to next */
};

/* Translation list. Translates a character code into up to 2
   indices of images to display on top of each other;
   '? -> index('a'), index('?) */
struct FONT_TRANSLIST {
	int16_t c0, c1;
};
struct FONT_TRANSINFO {
	uint16_t FirstChar;
	uint16_t LastChar;
	const FONT_TRANSLIST *pList;
};
struct FONT_MONO {
	const uint8_t *pData;
	const uint8_t *pTransData;
	const FONT_TRANSINFO *pTrans;
	uint16_t FirstChar, LastChar;
	uint8_t XSize;
	uint8_t XDist;
	uint8_t BytesPerLine;
};

/*
	  ****************************************
	  *                                      *
	  *      FONT info structure             *
	  *                                      *
	  ****************************************

This structure is used when retrieving information about a font.
It is designed for future expansion without incompatibilities.
*/
typedef struct {
	uint16_t Flags;
	uint8_t Baseline;
	uint8_t LHeight;     /* height of a small lower case character (a,x) */
	uint8_t CHeight;     /* height of a small upper case character (A,X) */
} GUI_FONTINFO;

#define GUI_FONTINFO_FLAG_PROP (1<<0)    /* Is proportional */
#define GUI_FONTINFO_FLAG_MONO (1<<1)    /* Is monospaced */

typedef uint16_t  tGUI_GetCharCode(const char  *s);
typedef int  tGUI_GetCharSize(const char  *s);
typedef int  tGUI_CalcSizeOfChar(uint16_t Char);
typedef int  tGUI_Encode(char *s, uint16_t Char);

typedef struct {
	tGUI_GetCharCode *pfGetCharCode;
	tGUI_GetCharSize *pfGetCharSize;
	tGUI_CalcSizeOfChar *pfCalcSizeOfChar;
	tGUI_Encode *pfEncode;
} GUI_UC_ENC_APILIST;

typedef int  tGUI_GetLineDistX(const char  *s, int Len);
typedef int  tGUI_GetLineLen(const char  *s, int MaxLen);
typedef void tGL_DispLine(const char  *s, int Len);

typedef struct {
	tGUI_GetLineDistX *pfGetLineDistX;
	tGUI_GetLineLen *pfGetLineLen;
	tGL_DispLine *pfDispLine;
} tGUI_ENC_APIList;

/*
	  ****************************************
	  *                                      *
	  *      FONT methods                    *
	  *                                      *
	  ****************************************

The parameter to the methods called pFont should be of type
FONT, but unfortunately a lot of compilers can not handle
forward declarations right ...
So it ends up to be a void pointer.
*/

struct FONT;
using CFONT = const FONT;
using PCFONT = const FONT *;

typedef void GUI_DISPCHAR(uint16_t c);
typedef int  GUI_GETCHARDISTX(uint16_t c);
typedef void GUI_GETFONTINFO(PCFONT pFont, GUI_FONTINFO *pfi);
typedef char GUI_ISINFONT(PCFONT pFont, uint16_t c);

#define DECLARE_FONT(Type)                                     \
void GUI##Type##_DispChar    (uint16_t c);                         \
int  GUI##Type##_GetCharDistX(uint16_t c);                         \
void GUI##Type##_GetFontInfo (PCFONT pFont, GUI_FONTINFO * pfi); \
char GUI##Type##_IsInFont    (PCFONT pFont, uint16_t c)

	/* MONO: Monospaced fonts */
	DECLARE_FONT(MONO);
#define GUI_FONTTYPE_MONO       \
  GUIMONO_DispChar,             \
	GUIMONO_GetCharDistX,         \
	GUIMONO_GetFontInfo,          \
	GUIMONO_IsInFont,             \
  (tGUI_ENC_APIList*)0

	/* PROP: Proportional fonts */
	DECLARE_FONT(PROP);
#define GUI_FONTTYPE_PROP       \
  GUIPROP_DispChar,             \
	GUIPROP_GetCharDistX,         \
	GUIPROP_GetFontInfo,          \
	GUIPROP_IsInFont,             \
  (tGUI_ENC_APIList*)0


struct FONT {
	GUI_DISPCHAR *pfDispChar;
	GUI_GETCHARDISTX *pfGetCharDistX;
	GUI_GETFONTINFO *pfGetFontInfo;
	GUI_ISINFONT *pfIsInFont;
	const tGUI_ENC_APIList *pafEncode;
	uint8_t YSize;
	uint8_t YDist;
	union {
		const void *pVoid;
		const FONT_MONO  *pMono;
		const FONT_PROP  *pProp;
	} p;
	uint8_t Baseline;
	uint8_t LHeight;     /* height of a small lower case character (a,x) */
	uint8_t CHeight;     /* height of a small upper case character (A,X) */
};



typedef void *GUI_HMEM, *GUI_HWIN;
