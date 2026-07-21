#pragma once

#include "WM_Intern.h"  /* Window manager, including some internals, which speed things up */

#include "SCROLLBAR.h"

#define WIDGET_STATE_FOCUS              (1<<0)
#define WIDGET_STATE_VERTICAL           (1<<3)
#define WIDGET_STATE_FOCUSSABLE         (1<<4)
#define WIDGET_STATE_USER0              (1<<8)    /* Freely available for derived widget */
#define WIDGET_STATE_USER1              (1<<9)    /* Freely available for derived widget */
#define WIDGET_STATE_USER2              (1<<10)   /* Freely available for derived widget */
#define WIDGET_ITEM_DRAW                0
#define WIDGET_ITEM_GET_XSIZE           1
#define WIDGET_ITEM_GET_YSIZE           2
#define WM_WIDGET_SET_EFFECT    WM_WIDGET + 0
#define WIDGET_CF_VERTICAL      WIDGET_STATE_VERTICAL

typedef struct {
	WM_HWIN hWin;
	int Cmd;         /* WIDGET_ITEM_GET_XSIZE, WIDGET_ITEM_GET_YSIZE, WIDGET_ITEM_DRAW, */
	int ItemIndex;
	int x0, y0;
} WIDGET_ITEM_DRAW_INFO;

typedef int WIDGET_DRAW_ITEM_FUNC(const WIDGET_ITEM_DRAW_INFO *pDrawItemInfo);

struct WIDGET_EFFECT {
	void(*pfDrawUp)(void);
	void(*pfDrawDown)(void);
	void(*pfDrawUpRect)  (GUI_RECT r);
	void(*pfDrawDownRect)(GUI_RECT r);
	GUI_RECT(*pfGetRect)();
	int EffectSize;
};

extern const WIDGET_EFFECT
	WIDGET_Effect_None,
	WIDGET_Effect_Simple,
	WIDGET_Effect_3D,
	WIDGET_Effect_3D1L,
	WIDGET_Effect_3D2L;

struct WIDGET : public WM_Obj {
	static const WIDGET_EFFECT *DefaultEffect;
	const WIDGET_EFFECT *pEffect = DefaultEffect;
	int16_t Id;
	uint16_t State;
};

/*********************************************************************
*
*         GUI_DRAW
*
* The GUI_DRAW object is used as base class for selfdrawing,
* non-windows objects. They are used as content of different widgets,
* such as the bitmap or header widgets.
*/
/* Declare Object struct */
typedef struct GUI_DRAW GUI_DRAW;
typedef void   GUI_DRAW_SELF_CB(void);

/* Declare Object constants (member functions etc)  */
struct GUI_DRAW_CONSTS {
	void (*pfDraw)    (const GUI_DRAW *pObj, int x, int y);
	int  (*pfGetXSize)(const GUI_DRAW *pObj);
	int  (*pfGetYSize)(const GUI_DRAW *pObj);
};

/* Declare Object */
struct GUI_DRAW {
	const GUI_DRAW_CONSTS *pConsts;
	union {
		const void *pData;
		GUI_DRAW_SELF_CB *pfDraw;
	} Data;
	int16_t xOff, yOff;
};

/* GUI_DRAW_ API */
void GUI_DRAW__Draw(GUI_DRAW *pDrawObj, int x, int y);
int  GUI_DRAW__GetXSize(GUI_DRAW *pDrawObj);
int  GUI_DRAW__GetYSize(GUI_DRAW *pDrawObj);

/* GUI_DRAW_ Constructurs for different objects */
GUI_DRAW *GUI_DRAW_BITMAP_Create(PCBITMAP pBitmap, int x, int y);
GUI_DRAW *GUI_DRAW_SELF_Create(GUI_DRAW_SELF_CB *pfDraw, int x, int y);

void      WIDGET__DrawFocusRect(WIDGET *pWidget, GUI_RECT r, int Dist);
void      WIDGET__DrawVLine(WIDGET *pWidget, int x, int y0, int y1);
void      WIDGET__FillRect(WIDGET *pWidget, GUI_RECT r);

void      WIDGET__EFFECT_DrawDownRect(WIDGET *pWidget, GUI_RECT r);
void      WIDGET__EFFECT_DrawDown(WIDGET *pWidget);
void      WIDGET__EFFECT_DrawUpRect(WIDGET *pWidget, GUI_RECT r);

RGBC WIDGET__GetBkColor(WM_HWIN hObj);

int       WIDGET__GetWindowSizeX(WM_HWIN hWin);

int       WIDGET__GetXSize(const WIDGET *pWidget);
int       WIDGET__GetYSize(const WIDGET *pWidget);
GUI_RECT  WIDGET__GetClientRect(WIDGET *pWidget);
GUI_RECT  WIDGET__GetInsideRect(WIDGET *pWidget);

void      WIDGET__Init(WIDGET *pWidget, int Id, uint16_t State);
void      WIDGET__RotateRect90(WIDGET *pWidget, GUI_RECT *pDest, const GUI_RECT *pRect);
void      WIDGET__SetScrollState(WM_HWIN hWin, const WM_SCROLL_STATE *pVState, const WM_SCROLL_STATE *pState);
void      WIDGET__FillStringInRect(const char *pText, GUI_RECT FillRect, GUI_RECT TextRectMax, GUI_RECT TextRectAct);

void  WIDGET_SetState(WM_HWIN hObj, int State);
void  WIDGET_AndState(WM_HWIN hObj, int State);
void  WIDGET_OrState(WM_HWIN hObj, int State);
int   WIDGET_GetState(WM_HWIN hObj);
int   WIDGET_SetWidth(WM_HWIN hObj, int Width);
int   WIDGET_SetWidth(WM_HWIN hObj, int Width);

void  WIDGET_SetEffect(WM_HWIN hObj, const WIDGET_EFFECT *pEffect);

int   WIDGET_HandleActive(WM_HWIN hObj, int MsgId, WM_PARAM *Data);
