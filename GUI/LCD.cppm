export module TUX.LCD;

export import TUX.Types;

export {
	
struct LCDDEV {
	PCLOGPALETTE pPal = nullptr;

	LCDDEV() {}
	virtual ~LCDDEV() {}

	virtual RECT Rect() const = 0;
	virtual BPP_MODE BitsPerPixel() const = 0;

	virtual void FillRect(RECT r, RGBC color) {
		for (int y = r.y0; y <= r.y1; ++y)
		for (int x = r.x0; x <= r.x1; ++x)
		SetPixel(x, y, color);
	}

	virtual RGBC GetPixel(int16_t x, int16_t y) = 0;
	virtual void SetPixel(int16_t x, int16_t y, RGBC color) = 0;
	void SetPixelPal(int16_t x, int16_t y, RGBC color) {
		SetPixel(x, y, pPal[color]);
	}
	void SetPixelTrans(int16_t x, int16_t y, RGBC color) {
		if (color)
			SetPixel(x, y, color);
	}
	void SetPixelTransPal(int16_t x, int16_t y, RGBC color) {
		if (color)
			SetPixel(x, y, pPal[color]);
	}

private:
	using _SetPixelFunc = void(LCDDEV:: *)(int16_t, int16_t, RGBC);
	void _DrawBitmapBytes(_SetPixelFunc _SetPixel,
						  RECT r, uint8_t BitsPerPixel,
						  const void *pData, uint16_t BytesPerLine) {
		uint8_t BytesPerPixel = BitsPerPixel / 8;
		static uint32_t(*const aGetPixel[])(const void *) {
			[](const void *pDat) -> uint32_t { return *(const uint8_t *)pDat; },
			[](const void *pDat) -> uint32_t { return *(const uint16_t *)pDat; }, // Order by endian
			[](const void *pDat) -> uint32_t { return *(const RGB24b *)pDat; },
			[](const void *pDat) -> uint32_t { return *(const RGB32b *)pDat; }
		};
		auto _GetPixel = aGetPixel[BytesPerPixel - 1];
		auto pPixel = (const uint8_t *)pData;
		for (auto y = r.y0; y <= r.y1; ++y) {
			auto pLine = pPixel;
			for (auto x = r.x0; x <= r.x1; ++x) {
				auto Pixel = _GetPixel(pLine);
				(this->*_SetPixel)(x, y, Pixel);
				pLine += BytesPerPixel;
			}
			pPixel += BytesPerLine;
		}
	}
	void _DrawBitmapBits(_SetPixelFunc _SetPixel,
						 RECT r, uint8_t BitsPerPixel,
						 const void *pData, uint16_t BytesPerLine,
						 uint8_t diff) {
		const uint8_t Mask = (1u << BitsPerPixel) - 1;
		auto pPixel = (const uint8_t *)pData;
		for (int y = r.y0; y <= r.y1; ++y) {
			uint32_t xBits = diff;
			for (int x = r.x0; x <= r.x1; ++x) {
				auto pixel = (pPixel[xBits >> 3] >> (xBits & 7)) & Mask;
				(this->*_SetPixel)(x, y, pixel);
				xBits += BitsPerPixel;
			}
			pPixel += BytesPerLine;
		}
	}
public:
	virtual void SetBitmap(BITVIEW b, bool HasTrans) {
		static const _SetPixelFunc aSetPixelFunc[]{
			&LCDDEV::SetPixel,
			&LCDDEV::SetPixelPal,
			&LCDDEV::SetPixelTrans,
			&LCDDEV::SetPixelTransPal
		};
		pPal = b.pPalEntries;
		auto _SetPix = aSetPixelFunc[(pPal ? 1 : 0) | (HasTrans ? 2 : 0)];
		if (auto bpp = BPP_Bits[b.BitsPerPixel]; bpp < 8)
			_DrawBitmapBits(_SetPix, b, bpp, b.pData, b.BytesPerLine, b.BitsXOff);
		else
			_DrawBitmapBytes(_SetPix, b, bpp, b.pData, b.BytesPerLine);
		pPal = nullptr;
	}
	virtual void GetBitmap(BITVIEW &b) {

	}
};

void LCD_DrawBitmap(BITVIEW b);
void LCD_SetPixel(int x, int y, RGBC Color);
void LCD_FillRect(RECT r);

}
