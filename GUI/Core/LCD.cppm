export module TUX.LCD;

export import TUX.Types;

export {
	
struct LCDDEV_API {
	uint8_t BitsPerPixel;
	PCLOGPALETTE pPal = nullptr;
	LCDDEV_API *pMemDevAPI = nullptr;

	LCDDEV_API(LCDDEV_API *pMemDevAPI, uint8_t BitsPerPixel)
		: BitsPerPixel(BitsPerPixel), pMemDevAPI(pMemDevAPI) {}

	virtual RECT GetRect() = 0;
	virtual RGBC GetPixel(int16_t x, int16_t y) = 0;
	virtual void FillRect(RECT r, RGBC color) {
		for (int y = r.y0; y <= r.y1; ++y)
			for (int x = r.x0; x <= r.x1; ++x)
				SetPixel(x, y, color);
	}

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
	using _SetPixelFunc = void(LCDDEV_API:: *)(int16_t, int16_t, RGBC);
	void _DrawBitmapBytes(_SetPixelFunc _SetPixel,
						  RECT r, uint8_t BitsPerPixel,
						  const void *pData, uint16_t BytesPerLine) {
		uint8_t BytesPerPixel = BitsPerPixel / 8;
		static uint32_t(*const aGetPixel[])(const void *) {
			[](const void *pDat) -> uint32_t { return *(const uint8_t *)pDat; },
			[](const void *pDat) -> uint32_t { return *(const uint16_t *)pDat; },
			[](const void *pDat) -> uint32_t { return *(const RGB24b *)pDat; },
			[](const void *pDat) -> uint32_t { return *(const uint32_t *)pDat; }
		};
		auto _GetPixel = aGetPixel[BytesPerPixel];
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
		const uint8_t PixelsPerByte = 8 / BitsPerPixel;
		const uint8_t Mask = 0xFF >> (8 - BitsPerPixel);
		auto pPixels = (const uint8_t *)pData;
		for (auto y = r.y0; y <= r.y1; ++y) {
			auto pLine = pPixels;
			auto Pixels = *pLine;
			auto Diff = diff;
			for (auto x = r.x0; x <= r.x1; ++x) {
				(this->*_SetPixel)(x, y, Pixels & Mask);
				if (++Diff < PixelsPerByte)
					Pixels >>= BitsPerPixel;
				else {
					Diff = 0;
					Pixels = *++pLine;
				}
			}
			pPixels += BytesPerLine;
		}
	}
public:
	virtual void DrawBitmap(BITVIEW b, bool HasTrans) {
		static const _SetPixelFunc aSetPixelFunc[]{
			&LCDDEV_API::SetPixel,
			&LCDDEV_API::SetPixelPal,
			&LCDDEV_API::SetPixelTrans,
			&LCDDEV_API::SetPixelTransPal
		};
		pPal = b.pPalEntries;
		auto _SetPix = aSetPixelFunc[(pPal ? 1 : 0) | (HasTrans ? 2 : 0)];
		if (b.BitsPerPixel < 8)
			_DrawBitmapBits(_SetPix, b, b.BitsPerPixel, b.pData, b.BytesPerLine, b.BitsXOff);
		else
			_DrawBitmapBytes(_SetPix, b, b.BitsPerPixel, b.pData, b.BytesPerLine);
		pPal = nullptr;
	}
};

}
