#pragma once

#include <stdlib.h>

#include "LCD.h"
#include "GUIConf.h"

typedef const char *GUI_ConstString;

typedef struct {
	void(*pfDraw)(int x0, int y0, int xsize, int ysize, const uint8_t  *pPixel, const GUI_LOGPALETTE  *pLogPal, int xMag, int yMag);
} BITMAP_METHODS;

struct GUI_BITMAP {
	uint16_t XSize, YSize;
	uint16_t BytesPerLine;
	uint16_t BitsPerPixel;
	const void *pData;
	const GUI_LOGPALETTE *pPal;
};
using CBITMAP = const GUI_BITMAP;
using PCBITMAP = const GUI_BITMAP *;

struct CURSOR {
	PCBITMAP pBitmap;
	int xHot, yHot;
};
using CCURSOR = const CURSOR;
using PCCURSOR = const CURSOR *;

struct GUI_PID_STATE {
	int16_t x, y;
	uint8_t Pressed;
};

#pragma region Encoder
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
#pragma endregion

#pragma region Font
struct FONT {
	uint8_t YSize, YDist;
	uint8_t Baseline, LHeight, CHeight;
	const tGUI_ENC_APIList *pafEncode = nullptr;

	FONT(uint8_t YSize, uint8_t YDist,
		 uint8_t Baseline,
		 uint8_t LHeight, uint8_t CHeight) :
		YSize(YSize), YDist(YDist),
		Baseline(Baseline),
		LHeight(LHeight), CHeight(CHeight) {}

	virtual void DispChar(uint16_t c) const = 0;
	virtual int  GetCharDistX(uint16_t c) const = 0;
	virtual bool IsInFont(uint16_t c) const = 0;
};
using CFONT = const FONT;
using PCFONT = const FONT *;

struct FONT_MONO : FONT {
	const void *pData;
	const void *pTransData;
	struct TRANSINFO {
		uint16_t FirstChar, LastChar;
		struct LIST {
			int16_t c0, c1;
		} const *pList;
	} const *pTrans;
	uint16_t FirstChar, LastChar;
	uint8_t XSize, XDist;
	uint8_t BytesPerLine;

	FONT_MONO(uint8_t YSize, uint8_t YDist,
			  uint8_t Baseline,
			  uint8_t LHeight, uint8_t CHeight,
			  /* For FONT_MONO */
			  const void *pData,
			  const void *pTransData,
			  const TRANSINFO *pTrans,
			  uint16_t FirstChar, uint16_t LastChar,
			  uint8_t XSize, uint8_t XDist,
			  uint8_t BytesPerLine) :
		FONT(YSize, YDist, Baseline, LHeight, CHeight),
		pData(pData), pTransData(pTransData), pTrans(pTrans),
		FirstChar(FirstChar), LastChar(LastChar),
		XSize(XSize), XDist(XDist),
		BytesPerLine(BytesPerLine) {}

	void DispChar(uint16_t c) const override;
	int  GetCharDistX(uint16_t c) const override;
	bool IsInFont(uint16_t c) const override;
};
using CFONT_MONO = const FONT_MONO;

struct FONT_PROP : FONT {
	uint16_t First, Last;
	struct CHARINFO {
		uint8_t XSize, XDist;
		uint8_t BytesPerLine;
		const void *pData;
	} const *paCharInfo;
	const FONT_PROP *pNext;

	FONT_PROP(uint8_t YSize, uint8_t YDist,
			  uint8_t Baseline,
			  uint8_t LHeight, uint8_t CHeight,
			  /* for FONT_PROP */
			  uint16_t First, uint16_t Last,
			  const CHARINFO *paCharInfo,
			  const FONT_PROP *pNext = nullptr) :
		FONT(YSize, YDist, Baseline, LHeight, CHeight),
		First(First), Last(Last),
		paCharInfo(paCharInfo),
		pNext(pNext) {}

	const FONT_PROP *FindChar(uint16_t c) const;

	void DispChar(uint16_t c) const override;
	int  GetCharDistX(uint16_t c) const override;
	bool IsInFont(uint16_t c) const override;
};
using CFONT_PROP = const FONT_PROP;

#pragma endregion

#pragma region Resources
extern CFONT_PROP GUI_Font8_ASCII, GUI_Font8_1;
extern CFONT_PROP GUI_Font13_ASCII, GUI_Font13_1;

extern CFONT_MONO GUI_Font8x8, GUI_Font8x9;
extern CFONT_MONO GUI_Font6x8, GUI_Font6x9;

extern CCURSOR GUI_CursorArrowS, GUI_CursorArrowSI;
extern CCURSOR GUI_CursorArrowM, GUI_CursorArrowMI;
extern CCURSOR GUI_CursorArrowL, GUI_CursorArrowLI;
extern CCURSOR GUI_CursorCrossS, GUI_CursorCrossSI;
extern CCURSOR GUI_CursorCrossM, GUI_CursorCrossMI;
extern CCURSOR GUI_CursorCrossL, GUI_CursorCrossLI;
extern CCURSOR GUI_CursorHeaderM, GUI_CursorHeaderMI;

extern CBITMAP GUI_BitmapArrowS, GUI_BitmapArrowSI;
extern CBITMAP GUI_BitmapArrowM, GUI_BitmapArrowMI;
extern CBITMAP GUI_BitmapArrowL, GUI_BitmapArrowLI;
extern CBITMAP GUI_BitmapCrossS, GUI_BitmapCrossSI;
extern CBITMAP GUI_BitmapCrossM, GUI_BitmapCrossMI;
extern CBITMAP GUI_BitmapCrossL, GUI_BitmapCrossLI;
#pragma endregion

typedef void *GUI_HMEM, *GUI_HWIN;
