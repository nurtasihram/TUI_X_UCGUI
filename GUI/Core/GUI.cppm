module;

#include "GUI_ConfDefaults.h"

#include "LCD.h"
#include "GUIConf.h"

export module TUX;

export import TUX.Types;
export import TUX.Resources;

export {

typedef void *GUI_HMEM;

struct PID_STATE : POINT {
	uint8_t Pressed;
	auto operator=(const POINT &p) {
		this->x = p.x;
		this->y = p.y;
	}
};

using DRAWMODE = uint8_t; // 2bits
constexpr DRAWMODE DRAWMODE_NORMAL = 0,
				   DRAWMODE_TRANS  = 1 << 1,
				   DRAWMODE_REV    = 1 << 2;

using TEXTALIGN = uint8_t; // 4bits
constexpr TEXTALIGN
/* Text alignment flags, horizontal */
	TEXTALIGN_LEFT        = (0<<0),
	TEXTALIGN_RIGHT       = (1<<0),
	TEXTALIGN_CENTER      = (2<<0),
	TEXTALIGN_HORIZONTAL  = (3<<0),
	TEXTALIGN_HCENTER     = TEXTALIGN_CENTER,
/* Text alignment flags, vertical */
	TEXTALIGN_TOP         = (0<<2),
	TEXTALIGN_BOTTOM      = (1<<2),
	TEXTALIGN_BASELINE    = (2<<2),
	TEXTALIGN_VCENTER     = (3<<2),
	TEXTALIGN_VERTICAL    = TEXTALIGN_VCENTER;

struct GUI_CONTEXT {
	/* Variables in LCD module */
	RGBC aColor[2];
	RECT ClipRect;
	DRAWMODE DrawMode;
	/* Variables in GUICHAR module */
	PCFONT pAFont;
	const GUI_UC_ENC_APILIST *pUC_API; /* Unicode encoding API */
	POINT DispPos;
	DRAWMODE TextMode = 0;
	TEXTALIGN TextAlign = 0;
	/* Variables in WM module */
	const RECT *WM__pUserClipRect = nullptr;
	POINT Off;
	/* Variables in MEMDEV module (with memory devices only) */
#if GUI_SUPPORT_DEVICES
	const tLCDDEV_APIList *pDeviceAPI;  /* function pointers only */
	GUI_HMEM hDevData;
	RECT ClipRectPrev;
#endif
public:
	DRAWMODE SetDrawMode(DRAWMODE dm) {
		DRAWMODE OldDM = DrawMode;
		if ((DrawMode ^ dm) & DRAWMODE_REV) {
			RGBC temp = aColor[0];
			aColor[0] = aColor[1];
			aColor[1] = temp;
		}
		DrawMode = dm;
		return OldDM;
	}

	auto GetBkColor(void) { return aColor[0]; }
	void SetBkColor(RGBC color) { aColor[(DrawMode & DRAWMODE_REV) ? 1 : 0] = color; }

	auto GetColor(void) { return aColor[1]; }
	void SetColor(RGBC color) { aColor[(DrawMode & DRAWMODE_REV) ? 0 : 1] = color; }

	auto GetTextAlign(void) { return TextAlign; }
	void SetTextAlign(TEXTALIGN Align) { TextAlign = Align; }

	auto GetTextMode(void) { return TextMode; }
	void SetTextMode(int Mode) { TextMode = Mode; }

	auto GetFont(void) { return pAFont; }
	PCFONT SetFont(PCFONT pNewFont) {
		PCFONT pOldFont = pAFont;
		if (pNewFont)
			pAFont = pNewFont;
		return pOldFont;
	}

public:

} GUI;

}
