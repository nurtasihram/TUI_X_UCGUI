module;

#include "GUI_ConfDefaults.h"

#include "LCD.h"
#include "GUIConf.h"

export module TUX;

export import TUX.Types;
export import TUX.Resources;

export {

typedef void *GUI_HMEM;

struct GUI_PID_STATE {
	int16_t x, y;
	uint8_t Pressed;
};

using DRAWMODE = uint8_t;
constexpr DRAWMODE DRAWMODE_NORMAL = 0;
constexpr DRAWMODE DRAWMODE_TRANS  = 1 << 1;
constexpr DRAWMODE DRAWMODE_REV    = 1 << 2;

struct GUI_CONTEXT {
	/* Variables in LCD module */
	uint32_t aColor[2];
	GUI_RECT ClipRect;
	uint8_t DrawMode;
	/* Variables in GUICHAR module */
	PCFONT pAFont;
	const GUI_UC_ENC_APILIST *pUC_API; /* Unicode encoding API */
	GUI_POINT DispPos;
	int16_t TextMode, TextAlign;
	/* Variables in WM module */
	const GUI_RECT *WM__pUserClipRect;
	GUI_POINT Off;
	/* Variables in MEMDEV module (with memory devices only) */
#if GUI_SUPPORT_DEVICES
	const tLCDDEV_APIList *pDeviceAPI;  /* function pointers only */
	GUI_HMEM    hDevData;
	GUI_RECT    ClipRectPrev;
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

	RGBC GetBkColor(void) { return aColor[0]; }
	void SetBkColor(RGBC color) { aColor[(DrawMode & DRAWMODE_REV) ? 1 : 0] = color; }

	RGBC GetColor(void) { return aColor[1]; }
	void SetColor(RGBC color) { aColor[(DrawMode & DRAWMODE_REV) ? 0 : 1] = color; }

	int GetTextAlign(void) { return TextAlign; }
	void SetTextAlign(int Align) { TextAlign = Align; }

	int GetTextMode(void) { return TextMode; }
	void SetTextMode(int Mode) { TextMode = Mode; }

	PCFONT GetFont(void) {
		return pAFont;
	}
	PCFONT SetFont(PCFONT pNewFont) {
		PCFONT pOldFont = pAFont;
		if (pNewFont)
			pAFont = pNewFont;
		return pOldFont;
	}

public:

} GUI;


}
