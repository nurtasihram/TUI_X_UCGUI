export module TUX.Types;

export import <stdint.h>;

export {

auto Max(auto v0, auto v1) { return v0 > v1 ? v0 : v1; }
auto Min(auto v0, auto v1) { return v0 < v1 ? v0 : v1; }

#pragma region Coordinates
struct POINT {
	int16_t x, y;

	constexpr POINT(int a = 0) : x(a), y(a) {}
	constexpr POINT(int x, int y) :
		x(x), y(y) {}

	inline operator bool() const { return x | y; }
	inline POINT operator~() const { return{ y, x }; }
	inline POINT operator+(int p) const { return{ x + p, y + p }; }
	inline POINT operator-(int p) const { return{ x - p, y - p }; }
	inline POINT operator+(const POINT &pt) const { return{ x + pt.x, y + pt.y }; }
	inline POINT operator-(const POINT &pt) const { return{ x - pt.x, y - pt.y }; }
	inline POINT &operator+=(const POINT &pt) { x += pt.x, y += pt.y; return *this; }
	inline POINT &operator-=(const POINT &pt) { x -= pt.x, y -= pt.y; return *this; }
};
struct RECT {
	int16_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	
	constexpr RECT() {}
	constexpr RECT(POINT LeftTop, POINT RightBottom) :
		x0(LeftTop.x), y0(LeftTop.y), x1(RightBottom.x), y1(RightBottom.y) {}
	constexpr RECT(int x0, int y0, int x1, int y1) :
		x0(x0), y0(y0), x1(x1), y1(y1) {}

	inline POINT LeftTop() const { return{ x0, y0 }; }
	inline void LeftTop(POINT Pos) { x0 = Pos.x, y0 = Pos.y; }
	inline static RECT LeftTop(POINT Pos, POINT Size)
	{ return { Pos.x, Pos.y, Pos.x + Size.x - 1, Pos.y + Size.y - 1 }; }

	inline POINT RightBottom() const { return{ x1, y1 }; }
	inline void RightBottom(POINT Pos) { x1 = Pos.x, y1 = Pos.y; }

	inline auto XSize() const { return x1 - x0 + 1; }
	inline auto YSize() const { return y1 - y0 + 1; }
	inline auto Size() const { return POINT{ XSize(), YSize() }; }

	inline auto DistX() const { return x1 - x0; }
	inline auto DistY() const { return y1 - y0; }
	inline auto Dist() const { return POINT{ DistX(), DistY() }; }

	inline RECT Rotate90L(int16_t XSize) const
	{ return{ XSize - y1, x0, XSize - y0, x1 }; }
	inline RECT Rotate90R(int16_t YSize) const
	{ return{ y0, YSize - x1, y1, YSize - x0 }; }

	inline RECT operator~() const { return{ y0, x0, y1, x1 }; }

	inline RECT &operator+=(const POINT &pt) {
		x0 += pt.x, y0 += pt.y, x1 += pt.x, y1 += pt.y;
		return *this;
	}
	inline RECT operator+(const POINT &pt) const
	{ return{ x0 + pt.x, y0 + pt.y, x1 + pt.x, y1 + pt.y }; }
	inline RECT &operator-=(const POINT &pt) {
		x0 -= pt.x, y0 -= pt.y, x1 -= pt.x, y1 -= pt.y;
		return *this;
	}
	inline RECT operator-(const POINT &pt) const
	{ return{ x0 - pt.x, y0 - pt.y, x1 - pt.x, y1 - pt.y }; }

	inline RECT &operator/=(int dist) {
		x0 += dist, y0 += dist, x1 -= dist, y1 -= dist;
		return *this;
	}
	inline RECT operator/(int dist) const
	{ return{ x0 + dist, y0 + dist, x1 - dist, y1 - dist }; }
	inline RECT &operator*=(int dist) {
		x0 -= dist, y0 -= dist, x1 += dist, y1 += dist;
		return *this;
	}
	inline RECT operator*(int dist) const
	{ return{ x0 - dist, y0 - dist, x1 + dist, y1 + dist }; }

	inline RECT operator+(const RECT &r) const
	{ return{ x0 + r.x0, y0 + r.y0, x1 + r.x1, y1 + r.y1 }; }
	inline RECT &operator-=(const RECT &r) {
		x0 -= r.x0, y0 -= r.y0, x1 -= r.x1, y1 -= r.y1;
		return *this;
	}
	inline RECT operator-(const RECT &r) const
	{ return{ x0 - r.x0, y0 - r.y0, x1 - r.x1, y1 - r.y1 }; }

	inline RECT &operator&=(const RECT &r) {
		if (x0 < r.x0) x0 = r.x0;
		if (y0 < r.y0) y0 = r.y0;
		if (x1 > r.x1) x1 = r.x1;
		if (y1 > r.y1) y1 = r.y1;
		return *this;
	}
	inline RECT operator&(const RECT &r) const
	{ return{ Max(x0, r.x0), Max(y0, r.y0), Min(x1, r.x1), Min(y1, r.y1) }; }

	inline RECT &operator|=(const RECT &r) {
		if (x0 > r.x0) x0 = r.x0;
		if (y0 > r.y0) y0 = r.y0;
		if (x1 < r.x1) x1 = r.x1;
		if (y1 < r.y1) y1 = r.y1;
		return *this;
	}
	inline RECT operator|(const RECT &r) const
	{ return{ Min(x0, r.x0), Min(y0, r.y0), Max(x1, r.x1), Max(y1, r.y1) }; }

	bool operator<=(const RECT &r)
	{ return r.x0 <= x1 && r.y0 <= y1 && r.x1 >= x0 && r.y1 >= y0; }
	bool operator<=(const POINT &pt)
	{ return x0 <= pt.x && pt.x <= x1 && y0 <= pt.y && pt.y <= y1; }

	inline operator bool() const
	{ return x0 <= x1 && y0 <= y1; }
};
#pragma endregion

#pragma region Bitmaps 

#pragma region Standard Colors
using RGBC = uint32_t;
constexpr RGBC COLOR_RGB (uint8_t r, uint8_t g, uint8_t b) { return (b) | ((g) << 8) | ((r) << 16); }
constexpr RGBC RGB_GRAYL (uint8_t a) { return COLOR_RGB(a, a, a); }
constexpr RGBC RGB_BLUEL (uint8_t a) { return COLOR_RGB(0, 0, a); }
constexpr RGBC RGB_GREENL(uint8_t a) { return COLOR_RGB(0, a, 0); }
constexpr RGBC RGB_REDL  (uint8_t a) { return COLOR_RGB(a, 0, 0); }
constexpr RGBC
	RGB_BLACK       = RGB_GRAYL(0x00),
	RGB_DARKGRAY    = RGB_GRAYL(0x40),
	RGB_GRAY        = RGB_GRAYL(0x80),
	RGB_LIGHTGRAY   = RGB_GRAYL(0xD3),
	RGB_WHITE       = RGB_GRAYL(0xFF),
	RGB_BLUE        = COLOR_RGB(0x00, 0x00, 0xFF),
	RGB_GREEN       = COLOR_RGB(0x00, 0xFF, 0x00),
	RGB_RED         = COLOR_RGB(0xFF, 0x00, 0x00),
	RGB_CYAN        = COLOR_RGB(0x00, 0xFF, 0xFF),
	RGB_MAGENTA     = COLOR_RGB(0xFF, 0x00, 0xFF),
	RGB_YELLOW      = COLOR_RGB(0xFF, 0xFF, 0x00),
	RGB_LIGHTBLUE   = COLOR_RGB(0x80, 0x80, 0xFF),
	RGB_LIGHTGREEN  = COLOR_RGB(0x80, 0xFF, 0x80),
	RGB_LIGHTRED    = COLOR_RGB(0xFF, 0x80, 0x80),
	RGB_LIGHTCYAN   = COLOR_RGB(0x80, 0xFF, 0xFF),
	RGB_LIGHTMAGENT = COLOR_RGB(0xFF, 0x80, 0xFF),
	RGB_LIGHTYELLOW = COLOR_RGB(0xFF, 0xFF, 0x80),
	RGB_DARKBLUE    = COLOR_RGB(0x00, 0x00, 0x80),
	RGB_DARKGREEN   = COLOR_RGB(0x00, 0x80, 0x00),
	RGB_DARKRED     = COLOR_RGB(0x80, 0x00, 0x00),
	RGB_DARKCYAN    = COLOR_RGB(0x00, 0x80, 0x80),
	RGB_DARKMAGENTA = COLOR_RGB(0x80, 0x00, 0x80),
	RGB_DARKYELLOW  = COLOR_RGB(0x80, 0x80, 0x00),
	RGB_BROWN       = COLOR_RGB(0xA5, 0x2A, 0x2A),
	
	RGB_INVALID = ~0;      /* Invalid color - more than 24 bits */
#pragma endregion

using CLOGPALETTE = const RGBC[];
using PCLOGPALETTE = const RGBC *;

struct BITVIEW : RECT {
	const void* pData;
	PCLOGPALETTE pPalEntries;
	uint16_t BytesPerLine;
	uint8_t BitsPerPixel : 5;
	uint8_t BitsXOff : 3;
public:
	BITVIEW(RECT r,
			uint16_t BytesPerLine,
			uint8_t BitsPerPixel,
			const void* pData,
			PCLOGPALETTE pPalEntries = nullptr) :
		RECT(r),
		pData(pData),
		pPalEntries(pPalEntries),
		BytesPerLine(BytesPerLine),
		BitsPerPixel(BitsPerPixel),
		BitsXOff(0) {}
public:
	bool operator&=(RECT rClip) {
		auto ptOld = LeftTop();
		if (!RECT::operator&=(rClip))
			return false;
		auto Off = LeftTop() - ptOld;
		uint8_t DiffBits = 0;
		if (Off.x > 0) {
			auto xOffBits = BitsPerPixel * Off.x;
			DiffBits = xOffBits & 7;
			(const uint8_t *&)pData += xOffBits >> 3;
		}
		if (Off.y > 0)
			(const uint8_t*&)pData += BytesPerLine * Off.y;
		return true;
	}
};

struct BITMAP {
	const void* pData;
	PCLOGPALETTE pPalEntries;
	POINT Size;
	uint16_t BytesPerLine;
	uint8_t BitsPerPixel : 5;
	uint8_t BitsXOff : 3;
	BITMAP(POINT Size,
		   uint16_t BytesPerLine,
		   uint8_t BitsPerPixel,
		   const void *pData,
		   PCLOGPALETTE pPalEntries = nullptr) :
		pData(pData),
		pPalEntries(pPalEntries),
		Size(Size),
		BytesPerLine(BytesPerLine),
		BitsPerPixel(BitsPerPixel),
		BitsXOff(0) {}
public:
	BITVIEW At(POINT Pos) const {
		return {
			RECT::LeftTop(Pos, Size),
			BytesPerLine,
			BitsPerPixel,
			pData,
			pPalEntries
		};
	}
};
using CBITMAP = const BITMAP;
using PCBITMAP = const BITMAP *;

struct CURSOR {
	PCBITMAP pBitmap;
	POINT Hot{};
};
using CCURSOR = const CURSOR;
using PCCURSOR = const CURSOR *;

extern CLOGPALETTE GUI_CursorPal, GUI_CursorPalI;
#pragma endregion

#pragma region Font
struct FONT {
	uint8_t YSize, Baseline;

	FONT(uint8_t YSize, uint8_t Baseline) :
		YSize(YSize), Baseline(Baseline) {}

	virtual bool IsInFont(uint16_t c) const = 0;
	virtual int  GetCharSizeX(uint16_t c) const = 0;
	virtual void DispChar(uint16_t c) const = 0;
};
using CFONT = const FONT;
using UCFONT = const FONT &;
using PCFONT = const FONT *;

struct FONT_MONO : FONT {
	const void *pData;
	struct TRANSINFO {
		uint16_t FirstChar, LastChar;
		struct LIST {
			int16_t c0 = -1, c1 = -1;
		} const *pList;
	} const *pTrans;
	uint16_t FirstChar, LastChar;
	uint8_t XSize;

	FONT_MONO(uint8_t YSize, uint8_t Baseline,
			  /* For FONT_MONO */
			  const void *pData,
			  const TRANSINFO *pTrans,
			  uint16_t FirstChar, uint16_t LastChar,
			  uint8_t XSize) :
		FONT(YSize, Baseline),
		pData(pData), pTrans(pTrans),
		FirstChar(FirstChar), LastChar(LastChar),
		XSize(XSize) {}

	bool IsInFont(uint16_t c) const {
		if (FirstChar <= c && c <= LastChar)
			return true;
		else if (pTrans)
			if (pTrans->FirstChar <= c && c <= pTrans->LastChar)
				return true;
		return false;
	}
	int GetCharSizeX(uint16_t c) const
	{ return XSize; }
	void DispChar(uint16_t c) const override;
};
using CFONT_MONO = const FONT_MONO;

struct FONT_PROP : FONT {
	uint16_t First, Last;
	struct CHARINFO {
		const void *pData;
		uint8_t XSize;
		uint8_t BytesPerLine;
	} const *paCharInfo;
	const FONT_PROP *pNext;

	FONT_PROP(uint8_t YSize, uint8_t Baseline,
			  /* for FONT_PROP */
			  uint16_t First, uint16_t Last,
			  const CHARINFO *paCharInfo,
			  const FONT_PROP *pNext = nullptr) :
		FONT(YSize, Baseline),
		First(First), Last(Last),
		paCharInfo(paCharInfo),
		pNext(pNext) {}

	const FONT_PROP *FindChar(uint16_t c) const {
		for (auto i = this; i; i = i->pNext)
			if (i->First <= c && c <= i->Last)
				return i;
		return nullptr;
	}
	bool IsInFont(uint16_t c) const override
	{ return FindChar(c); }
	int GetCharSizeX(uint16_t c) const override {
		if (auto pProp = FindChar(c))
			return pProp->paCharInfo[c - pProp->First].XSize;
		return 0;
	}
	void DispChar(uint16_t c) const override;
};
using CFONT_PROP = const FONT_PROP;
#pragma endregion

#pragma region Standard Pixel Formats
enum BITS_BPP1 : uint8_t {
________________,XX______________,__XX____________,XXXX____________,____XX__________,XX__XX__________,__XXXX__________,XXXXXX__________,
______XX________,XX____XX________,__XX__XX________,XXXX__XX________,____XXXX________,XX__XXXX________,__XXXXXX________,XXXXXXXX________,
________XX______,XX______XX______,__XX____XX______,XXXX____XX______,____XX__XX______,XX__XX__XX______,__XXXX__XX______,XXXXXX__XX______,
______XXXX______,XX____XXXX______,__XX__XXXX______,XXXX__XXXX______,____XXXXXX______,XX__XXXXXX______,__XXXXXXXX______,XXXXXXXXXX______,
__________XX____,XX________XX____,__XX______XX____,XXXX______XX____,____XX____XX____,XX__XX____XX____,__XXXX____XX____,XXXXXX____XX____,
______XX__XX____,XX____XX__XX____,__XX__XX__XX____,XXXX__XX__XX____,____XXXX__XX____,XX__XXXX__XX____,__XXXXXX__XX____,XXXXXXXX__XX____,
________XXXX____,XX______XXXX____,__XX____XXXX____,XXXX____XXXX____,____XX__XXXX____,XX__XX__XXXX____,__XXXX__XXXX____,XXXXXX__XXXX____,
______XXXXXX____,XX____XXXXXX____,__XX__XXXXXX____,XXXX__XXXXXX____,____XXXXXXXX____,XX__XXXXXXXX____,__XXXXXXXXXX____,XXXXXXXXXXXX____,
____________XX__,XX__________XX__,__XX________XX__,XXXX________XX__,____XX______XX__,XX__XX______XX__,__XXXX______XX__,XXXXXX______XX__,
______XX____XX__,XX____XX____XX__,__XX__XX____XX__,XXXX__XX____XX__,____XXXX____XX__,XX__XXXX____XX__,__XXXXXX____XX__,XXXXXXXX____XX__,
________XX__XX__,XX______XX__XX__,__XX____XX__XX__,XXXX____XX__XX__,____XX__XX__XX__,XX__XX__XX__XX__,__XXXX__XX__XX__,XXXXXX__XX__XX__,
______XXXX__XX__,XX____XXXX__XX__,__XX__XXXX__XX__,XXXX__XXXX__XX__,____XXXXXX__XX__,XX__XXXXXX__XX__,__XXXXXXXX__XX__,XXXXXXXXXX__XX__,
__________XXXX__,XX________XXXX__,__XX______XXXX__,XXXX______XXXX__,____XX____XXXX__,XX__XX____XXXX__,__XXXX____XXXX__,XXXXXX____XXXX__,
______XX__XXXX__,XX____XX__XXXX__,__XX__XX__XXXX__,XXXX__XX__XXXX__,____XXXX__XXXX__,XX__XXXX__XXXX__,__XXXXXX__XXXX__,XXXXXXXX__XXXX__,
________XXXXXX__,XX______XXXXXX__,__XX____XXXXXX__,XXXX____XXXXXX__,____XX__XXXXXX__,XX__XX__XXXXXX__,__XXXX__XXXXXX__,XXXXXX__XXXXXX__,
______XXXXXXXX__,XX____XXXXXXXX__,__XX__XXXXXXXX__,XXXX__XXXXXXXX__,____XXXXXXXXXX__,XX__XXXXXXXXXX__,__XXXXXXXXXXXX__,XXXXXXXXXXXXXX__,
______________XX,XX____________XX,__XX__________XX,XXXX__________XX,____XX________XX,XX__XX________XX,__XXXX________XX,XXXXXX________XX,
______XX______XX,XX____XX______XX,__XX__XX______XX,XXXX__XX______XX,____XXXX______XX,XX__XXXX______XX,__XXXXXX______XX,XXXXXXXX______XX,
________XX____XX,XX______XX____XX,__XX____XX____XX,XXXX____XX____XX,____XX__XX____XX,XX__XX__XX____XX,__XXXX__XX____XX,XXXXXX__XX____XX,
______XXXX____XX,XX____XXXX____XX,__XX__XXXX____XX,XXXX__XXXX____XX,____XXXXXX____XX,XX__XXXXXX____XX,__XXXXXXXX____XX,XXXXXXXXXX____XX,
__________XX__XX,XX________XX__XX,__XX______XX__XX,XXXX______XX__XX,____XX____XX__XX,XX__XX____XX__XX,__XXXX____XX__XX,XXXXXX____XX__XX,
______XX__XX__XX,XX____XX__XX__XX,__XX__XX__XX__XX,XXXX__XX__XX__XX,____XXXX__XX__XX,XX__XXXX__XX__XX,__XXXXXX__XX__XX,XXXXXXXX__XX__XX,
________XXXX__XX,XX______XXXX__XX,__XX____XXXX__XX,XXXX____XXXX__XX,____XX__XXXX__XX,XX__XX__XXXX__XX,__XXXX__XXXX__XX,XXXXXX__XXXX__XX,
______XXXXXX__XX,XX____XXXXXX__XX,__XX__XXXXXX__XX,XXXX__XXXXXX__XX,____XXXXXXXX__XX,XX__XXXXXXXX__XX,__XXXXXXXXXX__XX,XXXXXXXXXXXX__XX,
____________XXXX,XX__________XXXX,__XX________XXXX,XXXX________XXXX,____XX______XXXX,XX__XX______XXXX,__XXXX______XXXX,XXXXXX______XXXX,
______XX____XXXX,XX____XX____XXXX,__XX__XX____XXXX,XXXX__XX____XXXX,____XXXX____XXXX,XX__XXXX____XXXX,__XXXXXX____XXXX,XXXXXXXX____XXXX,
________XX__XXXX,XX______XX__XXXX,__XX____XX__XXXX,XXXX____XX__XXXX,____XX__XX__XXXX,XX__XX__XX__XXXX,__XXXX__XX__XXXX,XXXXXX__XX__XXXX,
______XXXX__XXXX,XX____XXXX__XXXX,__XX__XXXX__XXXX,XXXX__XXXX__XXXX,____XXXXXX__XXXX,XX__XXXXXX__XXXX,__XXXXXXXX__XXXX,XXXXXXXXXX__XXXX,
__________XXXXXX,XX________XXXXXX,__XX______XXXXXX,XXXX______XXXXXX,____XX____XXXXXX,XX__XX____XXXXXX,__XXXX____XXXXXX,XXXXXX____XXXXXX,
______XX__XXXXXX,XX____XX__XXXXXX,__XX__XX__XXXXXX,XXXX__XX__XXXXXX,____XXXX__XXXXXX,XX__XXXX__XXXXXX,__XXXXXX__XXXXXX,XXXXXXXX__XXXXXX,
________XXXXXXXX,XX______XXXXXXXX,__XX____XXXXXXXX,XXXX____XXXXXXXX,____XX__XXXXXXXX,XX__XX__XXXXXXXX,__XXXX__XXXXXXXX,XXXXXX__XXXXXXXX,
______XXXXXXXXXX,XX____XXXXXXXXXX,__XX__XXXXXXXXXX,XXXX__XXXXXXXXXX,____XXXXXXXXXXXX,XX__XXXXXXXXXXXX,__XXXXXXXXXXXXXX,XXXXXXXXXXXXXXXX};
using BM_BPP1 = const BITS_BPP1;

enum BITS_BPP2 : uint8_t {
________,______XX,______oo,______dd,____XX__,____XXXX,____XXoo,____XXdd,
____oo__,____ooXX,____oooo,____oodd,____dd__,____ddXX,____ddoo,____dddd,
__XX____,__XX__XX,__XX__oo,__XX__dd,__XXXX__,__XXXXXX,__XXXXoo,__XXXXdd,
__XXoo__,__XXooXX,__XXoooo,__XXoodd,__XXdd__,__XXddXX,__XXddoo,__XXdddd,
__oo____,__oo__XX,__oo__oo,__oo__dd,__ooXX__,__ooXXXX,__ooXXoo,__ooXXdd,
__oooo__,__ooooXX,__oooooo,__oooodd,__oodd__,__ooddXX,__ooddoo,__oodddd,
__dd____,__dd__XX,__dd__oo,__dd__dd,__ddXX__,__ddXXXX,__ddXXoo,__ddXXdd,
__ddoo__,__ddooXX,__ddoooo,__ddoodd,__dddd__,__ddddXX,__ddddoo,__dddddd,
XX______,XX____XX,XX____oo,XX____dd,XX__XX__,XX__XXXX,XX__XXoo,XX__XXdd,
XX__oo__,XX__ooXX,XX__oooo,XX__oodd,XX__dd__,XX__ddXX,XX__ddoo,XX__dddd,
XXXX____,XXXX__XX,XXXX__oo,XXXX__dd,XXXXXX__,XXXXXXXX,XXXXXXoo,XXXXXXdd,
XXXXoo__,XXXXooXX,XXXXoooo,XXXXoodd,XXXXdd__,XXXXddXX,XXXXddoo,XXXXdddd,
XXoo____,XXoo__XX,XXoo__oo,XXoo__dd,XXooXX__,XXooXXXX,XXooXXoo,XXooXXdd,
XXoooo__,XXooooXX,XXoooooo,XXoooodd,XXoodd__,XXooddXX,XXooddoo,XXoodddd,
XXdd____,XXdd__XX,XXdd__oo,XXdd__dd,XXddXX__,XXddXXXX,XXddXXoo,XXddXXdd,
XXddoo__,XXddooXX,XXddoooo,XXddoodd,XXdddd__,XXddddXX,XXddddoo,XXdddddd,
oo______,oo____XX,oo____oo,oo____dd,oo__XX__,oo__XXXX,oo__XXoo,oo__XXdd,
oo__oo__,oo__ooXX,oo__oooo,oo__oodd,oo__dd__,oo__ddXX,oo__ddoo,oo__dddd,
ooXX____,ooXX__XX,ooXX__oo,ooXX__dd,ooXXXX__,ooXXXXXX,ooXXXXoo,ooXXXXdd,
ooXXoo__,ooXXooXX,ooXXoooo,ooXXoodd,ooXXdd__,ooXXddXX,ooXXddoo,ooXXdddd,
oooo____,oooo__XX,oooo__oo,oooo__dd,ooooXX__,ooooXXXX,ooooXXoo,ooooXXdd,
oooooo__,ooooooXX,oooooooo,oooooodd,oooodd__,ooooddXX,ooooddoo,oooodddd,
oodd____,oodd__XX,oodd__oo,oodd__dd,ooddXX__,ooddXXXX,ooddXXoo,ooddXXdd,
ooddoo__,ooddooXX,ooddoooo,ooddoodd,oodddd__,ooddddXX,ooddddoo,oodddddd,
dd______,dd____XX,dd____oo,dd____dd,dd__XX__,dd__XXXX,dd__XXoo,dd__XXdd,
dd__oo__,dd__ooXX,dd__oooo,dd__oodd,dd__dd__,dd__ddXX,dd__ddoo,dd__dddd,
ddXX____,ddXX__XX,ddXX__oo,ddXX__dd,ddXXXX__,ddXXXXXX,ddXXXXoo,ddXXXXdd,
ddXXoo__,ddXXooXX,ddXXoooo,ddXXoodd,ddXXdd__,ddXXddXX,ddXXddoo,ddXXdddd,
ddoo____,ddoo__XX,ddoo__oo,ddoo__dd,ddooXX__,ddooXXXX,ddooXXoo,ddooXXdd,
ddoooo__,ddooooXX,ddoooooo,ddoooodd,ddoodd__,ddooddXX,ddooddoo,ddoodddd,
dddd____,dddd__XX,dddd__oo,dddd__dd,ddddXX__,ddddXXXX,ddddXXoo,ddddXXdd,
ddddoo__,ddddooXX,ddddoooo,ddddoodd,dddddd__,ddddddXX,ddddddoo,dddddddd};
using BM_BPP2 = const BITS_BPP2;
#pragma endregion

}
