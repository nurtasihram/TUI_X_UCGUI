#pragma once

#include <stdlib.h>

#include "LCD.h"
#include "GUIConf.h"

typedef const char *GUI_ConstString;

typedef struct {
	void      (*pfDraw)(int x0, int y0, int xsize, int ysize, const uint8_t GUI_UNI_PTR *pPixel, const GUI_LOGPALETTE GUI_UNI_PTR *pLogPal, int xMag, int yMag);
} GUI_BITMAP_METHODS;

typedef struct {
	uint16_t XSize;
	uint16_t YSize;
	uint16_t BytesPerLine;
	uint16_t BitsPerPixel;
	const uint8_t GUI_UNI_PTR *pData;
	const GUI_LOGPALETTE GUI_UNI_PTR *pPal;
	const GUI_BITMAP_METHODS *pMethods;
} GUI_BITMAP;

typedef struct {
	int x, y;
	uint8_t Pressed;
} GUI_PID_STATE;

/*
	  ****************************************
	  *                                      *
	  *      FONT structures (new in V1.10)  *
	  *                                      *
	  ****************************************
*/

/* Translation list. Translates a character code into up to 2
   indices of images to display on top of each other;
   '? -> index('a'), index('?) */
typedef struct {
	int16_t c0;
	int16_t c1;
} GUI_FONT_TRANSLIST;

typedef struct {
	uint16_t FirstChar;
	uint16_t LastChar;
	const GUI_FONT_TRANSLIST GUI_UNI_PTR *pList;
} GUI_FONT_TRANSINFO;

typedef struct {
	uint8_t XSize;
	uint8_t XDist;
	uint8_t BytesPerLine;
	const uint8_t GUI_UNI_PTR *pData;
} GUI_CHARINFO;

typedef struct GUI_FONT_PROP {
	uint16_t First;                                /* first character               */
	uint16_t Last;                                 /* last character                */
	const GUI_CHARINFO GUI_UNI_PTR *paCharInfo;            /* address of first character    */
	const struct GUI_FONT_PROP GUI_UNI_PTR *pNext;        /* pointer to next */
} GUI_FONT_PROP;

typedef struct {
	const uint8_t GUI_UNI_PTR *pData;
	const uint8_t GUI_UNI_PTR *pTransData;
	const GUI_FONT_TRANSINFO GUI_UNI_PTR *pTrans;
	uint16_t FirstChar;
	uint16_t LastChar;
	uint8_t XSize;
	uint8_t XDist;
	uint8_t BytesPerLine;
} GUI_FONT_MONO;

typedef struct GUI_FONT_INFO {
	uint16_t First;                        /* first character               */
	uint16_t Last;                         /* last character                */
	const GUI_CHARINFO *paCharInfo;    /* address of first character    */
	const struct GUI_FONT_INFO *pNext; /* pointer to next */
} GUI_FONT_INFO;

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

typedef uint16_t  tGUI_GetCharCode(const char GUI_UNI_PTR *s);
typedef int  tGUI_GetCharSize(const char GUI_UNI_PTR *s);
typedef int  tGUI_CalcSizeOfChar(uint16_t Char);
typedef int  tGUI_Encode(char *s, uint16_t Char);

typedef struct {
	tGUI_GetCharCode *pfGetCharCode;
	tGUI_GetCharSize *pfGetCharSize;
	tGUI_CalcSizeOfChar *pfCalcSizeOfChar;
	tGUI_Encode *pfEncode;
} GUI_UC_ENC_APILIST;

typedef int  tGUI_GetLineDistX(const char GUI_UNI_PTR *s, int Len);
typedef int  tGUI_GetLineLen(const char GUI_UNI_PTR *s, int MaxLen);
typedef void tGL_DispLine(const char GUI_UNI_PTR *s, int Len);

typedef struct {
	tGUI_GetLineDistX *pfGetLineDistX;
	tGUI_GetLineLen *pfGetLineLen;
	tGL_DispLine *pfDispLine;
} tGUI_ENC_APIList;

extern const tGUI_ENC_APIList GUI_ENC_APIList_SJIS;

/*
	  ****************************************
	  *                                      *
	  *      FONT methods                    *
	  *                                      *
	  ****************************************

The parameter to the methods called pFont should be of type
GUI_FONT, but unfortunately a lot of compilers can not handle
forward declarations right ...
So it ends up to be a void pointer.
*/

typedef struct GUI_FONT GUI_FONT;

typedef void GUI_DISPCHAR(uint16_t c);
typedef int  GUI_GETCHARDISTX(uint16_t c);
typedef void GUI_GETFONTINFO(const GUI_FONT GUI_UNI_PTR *pFont, GUI_FONTINFO *pfi);
typedef char GUI_ISINFONT(const GUI_FONT GUI_UNI_PTR *pFont, uint16_t c);

#define DECLARE_FONT(Type)                                     \
void GUI##Type##_DispChar    (uint16_t c);                         \
int  GUI##Type##_GetCharDistX(uint16_t c);                         \
void GUI##Type##_GetFontInfo (const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO * pfi); \
char GUI##Type##_IsInFont    (const GUI_FONT GUI_UNI_PTR * pFont, uint16_t c)

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

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

	/* PROP: Proportional fonts SJIS */
	DECLARE_FONT(PROP);
#define GUI_FONTTYPE_PROP_SJIS  \
  GUIPROP_DispChar,             \
	GUIPROP_GetCharDistX,         \
	GUIPROP_GetFontInfo,          \
	GUIPROP_IsInFont,             \
  &GUI_ENC_APIList_SJIS

#if defined(__cplusplus)
}
#endif

struct GUI_FONT {
	GUI_DISPCHAR *pfDispChar;
	GUI_GETCHARDISTX *pfGetCharDistX;
	GUI_GETFONTINFO *pfGetFontInfo;
	GUI_ISINFONT *pfIsInFont;
	const tGUI_ENC_APIList *pafEncode;
	uint8_t YSize;
	uint8_t YDist;
	union {
		const void          GUI_UNI_PTR *pFontData;
		const GUI_FONT_MONO GUI_UNI_PTR *pMono;
		const GUI_FONT_PROP GUI_UNI_PTR *pProp;
	} p;
	uint8_t Baseline;
	uint8_t LHeight;     /* height of a small lower case character (a,x) */
	uint8_t CHeight;     /* height of a small upper case character (A,X) */
};

typedef struct {
	uint32_t ID;           /* Font file ID */
	uint16_t YSize;        /* Height of font */
	uint16_t YDist;        /* Space of font Y */
	uint16_t Baseline;     /* Index of baseline */
	uint16_t LHeight;      /* Height of a small lower case character (a) */
	uint16_t CHeight;      /* Height of a upper case character (A) */
	uint16_t NumAreas;     /* Number of character areas */
} GUI_SI_FONT;

typedef struct {
	uint16_t First;        /* Index of first character */
	uint16_t Last;         /* Index of last character */
} GUI_SIF_CHAR_AREA;

typedef struct {
	uint16_t XSize;        /* Size of bitmap data in X */
	uint16_t XDist;        /* Number of pixels for increment cursor in X */
	uint16_t BytesPerLine; /* Number of bytes per line */
	uint16_t Dummy;
	uint32_t OffData;      /* Offset of pixel data */
} GUI_SIF_CHARINFO;

typedef struct tGUI_SIF_APIList_struct {
	GUI_DISPCHAR *pDispChar;
	GUI_GETCHARDISTX *pGetCharDistX;
	GUI_GETFONTINFO *pGetFontInfo;
	GUI_ISINFONT *pIsInFont;
} tGUI_SIF_APIList;

#define GUI_SIF_TYPE      tGUI_SIF_APIList
#define GUI_SIF_TYPE_PROP &GUI_SIF_APIList_Prop

extern const tGUI_SIF_APIList GUI_SIF_APIList_Prop;

/*
	  *********************************
	  *                               *
	  *      Typedefs                 *
	  *                               *
	  *********************************
*/

typedef void *GUI_HMEM, *GUI_HWIN;
#define GUI_HMEM_NULL NULL
