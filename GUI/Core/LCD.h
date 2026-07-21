#pragma once

#include <stdint.h>
#include "GUI_ConfDefaults.h" /* Used for  */

#define DRAWMODE_NORMAL (0)
#define DRAWMODE_TRANS  (1<<1)
#define DRAWMODE_REV    (1<<2)

typedef int GUI_DRAWMODE;
typedef uint32_t RGBC;

inline auto Max(auto v0, auto v1) { return v0 > v1 ? v0 : v1; }
inline auto Min(auto v0, auto v1) { return v0 < v1 ? v0 : v1; }

struct GUI_POINT {
	int16_t x, y;

	constexpr GUI_POINT(int a = 0) : x(a), y(a) {}
	constexpr GUI_POINT(int x, int y) :
		x(x), y(y) {}

	inline GUI_POINT operator~() const { return{ y, x }; }
};
struct GUI_RECT {
	int16_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;

	constexpr GUI_RECT() {}
	constexpr GUI_RECT(int x0, int y0, int x1, int y1) :
		x0(x0), y0(y0), x1(x1), y1(y1) {}

	inline GUI_POINT LeftTop() const { return{ x0, y0 }; }

	inline auto XSize() const { return x1 - x0 + 1; }
	inline auto YSize() const { return y1 - y0 + 1; }

	inline GUI_RECT operator~() const { return{ y0, x0, y1, x1 }; }

	inline GUI_RECT &operator+=(const GUI_POINT &pt) {
		x0 += pt.x, y0 += pt.y, x1 += pt.x, y1 += pt.y;
		return *this;
	}
	inline GUI_RECT operator+(const GUI_POINT &pt) const
	{ return{ x0 + pt.x, y0 + pt.y, x1 + pt.x, y1 + pt.y }; }
	inline GUI_RECT &operator-=(const GUI_POINT &pt) {
		x0 -= pt.x, y0 -= pt.y, x1 -= pt.x, y1 -= pt.y;
		return *this;
	}
	inline GUI_RECT operator-(const GUI_POINT &pt) const
	{ return{ x0 - pt.x, y0 - pt.y, x1 - pt.x, y1 - pt.y }; }

	inline GUI_RECT &operator-=(int dist) {
		x0 += dist, y0 += dist, x1 -= dist, y1 -= dist;
		return *this;
	}
	inline GUI_RECT operator-(int dist) const
	{ return{ x0 + dist, y0 + dist, x1 - dist, y1 - dist }; }

	inline GUI_RECT &operator&=(const GUI_RECT &r) {
		if (x0 < r.x0) x0 = r.x0;
		if (y0 < r.y0) y0 = r.y0;
		if (x1 > r.x1) x1 = r.x1;
		if (y1 > r.y1) y1 = r.y1;
		return *this;
	}
	inline GUI_RECT operator&(const GUI_RECT &r) const 
	{ return{ Max(x0, r.x0), Max(y0, r.y0), Min(x1, r.x1), Min(y1, r.y1) }; }

	inline GUI_RECT &operator|=(const GUI_RECT &r) {
		if (x0 > r.x0) x0 = r.x0;
		if (y0 > r.y0) y0 = r.y0;
		if (x1 < r.x1) x1 = r.x1;
		if (y1 < r.y1) y1 = r.y1;
		return *this;
	}
	inline GUI_RECT operator|(const GUI_RECT &r) const 
	{ return{ Min(x0, r.x0), Min(y0, r.y0), Max(x1, r.x1), Max(y1, r.y1) }; }

	inline operator bool() const
	{ return x0 <= x1 && y0 <= y1; }
};

typedef struct {
	int  NumEntries;
	char HasTrans;
	const RGBC *pPalEntries;
} GUI_LOGPALETTE;

/* This is used for the simulation only ! */
typedef struct {
	int16_t x, y;
	uint8_t KeyStat;
} LCD_tMouseState;

/*********************************************************************
*
*      LCDDEV function table
*
**********************************************************************

  Below the routines which need to in an LCDDEV routine table are
  defined. All of these routines have to be in the low-level driver
  (LCD_L0) or in the memory device which can be used to replace the
  driver.
  The one exception to this is the SetClipRect routine, which would
  be identical for all drivers and is therefor contained in the
  level above (LCD).
*/
typedef void      tLCDDEV_DrawHLine(int x0, int y0, int x1);
typedef void      tLCDDEV_DrawVLine(int x, int y0, int y1);
typedef void      tLCDDEV_FillRect(int x0, int y0, int x1, int y1);
typedef RGBC tLCDDEV_GetPixel(int x, int y);
typedef void      tLCDDEV_SetPixel(int x, int y, RGBC Color);
typedef void      tLCDDEV_GetRect(GUI_RECT *pRect);

typedef void tLCDDEV_DrawBitmap(int x0, int y0, int xsize, int ysize,
								int BitsPerPixel, int BytesPerLine,
								const uint8_t *pData, int Diff,
								const void *pTrans);

typedef struct tLCDDEV_APIList_struct {
	tLCDDEV_DrawBitmap *pfDrawBitmap;
	tLCDDEV_DrawHLine *pfDrawHLine;
	tLCDDEV_DrawVLine *pfDrawVLine;
	tLCDDEV_FillRect *pfFillRect;
	tLCDDEV_GetPixel *pfGetPixel;
	tLCDDEV_GetRect *pfGetRect;
	tLCDDEV_SetPixel *pfSetPixel;
#if GUI_SUPPORT_MEMDEV
	const struct tLCDDEV_APIList_struct *pMemDevAPI;
	unsigned BitsPerPixel;
#endif
} tLCDDEV_APIList;

extern const tLCDDEV_APIList GUI_MEMDEV__APIList24;

void LCD_DrawHLine(int x0, int y0, int x1);
void LCD_DrawPixel(int x0, int y0);
void LCD_DrawVLine(int x, int y0, int y1);

void LCD_SetClipRectEx(const GUI_RECT *pRect);
void LCD_SetClipRectMax(void);

void LCD_SetPixel(int x, int y, int Color);
RGBC LCD_GetPixel(int x, int y);

void LCD_FillRect(int x0, int y0, int x1, int y1);

void LCD_X_Init(void);
