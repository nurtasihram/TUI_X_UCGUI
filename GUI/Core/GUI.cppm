module;

#include "GUI_ConfDefaults.h"

#include "LCD.h"
#include "GUIConf.h"

export module TUX;

export import TUX.Types;
export import TUX.Resources;
export import TUX.String;

export {

struct PID_STATE : POINT {
	int8_t Pressed;
	PID_STATE(POINT Pos, int8_t Pressed = 0) : POINT(Pos), Pressed(Pressed) {}
	auto operator=(const POINT &p) { x = p.x, y = p.y; }
	bool operator==(const PID_STATE &p) const
	{ return x == p.x && y == p.y &&	Pressed == p.Pressed; }
	bool operator!=(const PID_STATE &p) const
	{ return !(*this == p); }
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
	TEXTALIGN_HCENTER     = (2<<0),
	TEXTALIGN_HORIZONTAL  = (3<<0),
/* Text alignment flags, vertical */
	TEXTALIGN_TOP         = (0<<2),
	TEXTALIGN_BOTTOM      = (1<<2),
	TEXTALIGN_VCENTER     = (2<<2),
	TEXTALIGN_VERTICAL    = (3<<2);

#if GUI_SUPPORT_DEVICES

struct GUI_MEMDEV {
	int16_t x0, y0, XSize, YSize;
	int16_t BytesPerLine;
	int16_t BitsPerPixel;
	const tLCDDEV_APIList *pAPIList;
};

typedef void GUI_CALLBACK_VOID_P(void *p);

void GUI_MEMDEV__CopyFromLCD(GUI_MEMDEV *pDev);
void GUI_MEMDEV__GetRect(RECT *pRect);

GUI_MEMDEV *GUI_MEMDEV__CreateFixed(int x0, int y0, int xsize, int ysize, int Flags,
										  const tLCDDEV_APIList *pMemDevAPI);

void *GUI_MEMDEV__XY2PTR(int x, int y);
void *GUI_MEMDEV__XY2PTREx(GUI_MEMDEV *pDev, int x, int y);
void  GUI_MEMDEV__WriteToActiveAt(GUI_MEMDEV *pDev, int x, int y);

/* Create a memory device which is compatible to the selected LCD */
GUI_MEMDEV *GUI_MEMDEV_Create(int x0, int y0, int XSize, int YSize);
GUI_MEMDEV *GUI_MEMDEV_CreateEx(int x0, int y0, int XSize, int YSize, int Flags);
GUI_MEMDEV *GUI_MEMDEV_CreateFixed(int x0, int y0, int xsize, int ysize, int Flags,
										 const tLCDDEV_APIList *pMemDevAPI);
void GUI_MEMDEV_Clear(GUI_MEMDEV *pDev);
void GUI_MEMDEV_CopyFromLCD(GUI_MEMDEV *pDev);
void GUI_MEMDEV_CopyToLCD(GUI_MEMDEV *pDev);
void GUI_MEMDEV_CopyToLCDAt(GUI_MEMDEV *pDev, int x, int y);
void GUI_MEMDEV_Delete(GUI_MEMDEV *pDev);
int  GUI_MEMDEV_GetXSize(GUI_MEMDEV *pDev);
int  GUI_MEMDEV_GetYSize(GUI_MEMDEV *pDev);
void GUI_MEMDEV_ReduceYSize(GUI_MEMDEV *pDev, int YSize);
GUI_MEMDEV *GUI_MEMDEV_Select(GUI_MEMDEV *pDev);  /* Select (activate) a particular memory device. */
void  GUI_MEMDEV_SetOrg(GUI_MEMDEV *pDev, int x0, int y0);
int   GUI_MEMDEV_Draw(RECT *pRect, GUI_CALLBACK_VOID_P *pfDraw, void *pData, int MemSize, int Flags);
#endif

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
	/* Variables in WM module */
	const RECT *WM__pUserClipRect = nullptr;
	POINT Off;
	/* Variables in MEMDEV module (with memory devices only) */
#if GUI_SUPPORT_DEVICES
	const tLCDDEV_APIList *pDeviceAPI;  /* function pointers only */
	GUI_MEMDEV *pDevData;
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

	auto BkColor() const { return aColor[0]; }
	void BkColor(RGBC color) { aColor[(DrawMode & DRAWMODE_REV) ? 1 : 0] = color; }

	auto Color() const { return aColor[1]; }
	void Color(RGBC color) { aColor[(DrawMode & DRAWMODE_REV) ? 0 : 1] = color; }

	auto GetTextMode() const { return TextMode; }
	void SetTextMode(int Mode) { TextMode = Mode; }

	UCFONT Font() const { return *pAFont; } /// 
	PCFONT Font(PCFONT pNewFont) {
		PCFONT pOldFont = pAFont;
		if (pNewFont)
			pAFont = pNewFont;
		return pOldFont;
	}

public:

} GUI;

}
