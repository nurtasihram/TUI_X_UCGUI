#include "GUI_Protected.h"

#include "BUTTON.h"
#include "BUTTON_Private.h"

/* Define default fonts */
#define BUTTON_FONT_DEFAULT &GUI_Font13_1
#define BUTTON_USE_3D 1
#define BUTTON_3D_MOVE_X 1
#define BUTTON_3D_MOVE_Y 1
/* Define colors */
#define BUTTON_BKCOLOR0_DEFAULT   RGB_GRAYL(0xAA)
#define BUTTON_BKCOLOR1_DEFAULT   RGB_WHITE
#define BUTTON_BKCOLOR2_DEFAULT   RGB_LIGHTGRAY
#define BUTTON_TEXTCOLOR0_DEFAULT RGB_BLACK
#define BUTTON_TEXTCOLOR1_DEFAULT RGB_BLACK
#define BUTTON_TEXTCOLOR2_DEFAULT RGB_DARKGRAY
#define BUTTON_REACT_ON_LEVEL 0
#define BUTTON_ALIGN_DEFAULT GUI_TA_HCENTER | GUI_TA_VCENTER
BUTTON_PROPS BUTTON__DefaultProps = {
  BUTTON_BKCOLOR0_DEFAULT,
  BUTTON_BKCOLOR1_DEFAULT,
  BUTTON_BKCOLOR2_DEFAULT,
  BUTTON_TEXTCOLOR0_DEFAULT,
  BUTTON_TEXTCOLOR1_DEFAULT,
  BUTTON_TEXTCOLOR2_DEFAULT,
  BUTTON_FONT_DEFAULT,
  BUTTON_ALIGN_DEFAULT
};
static void _OnPaint(BUTTON_Obj *pObj) {
	const char *s = NULL;
	unsigned int Index;
	int State, PressedState, ColorIndex;
	GUI_RECT rClient, rInside;
	State = pObj->Widget.State;
	PressedState = (State & BUTTON_STATE_PRESSED) ? 1 : 0;
	ColorIndex = (WM_IsEnabled(pObj)) ? PressedState : 2;
	GUI_SetFont(pObj->Props.pFont);
	if (pObj->hpText) {
		s = (const char *)(pObj->hpText);
	}
	WM_GetClientRect(&rClient);
	/* Start drawing */
	rInside = rClient;
	/* Draw the 3D effect (if configured) */
#if BUTTON_USE_3D
	{
		int EffectSize;
		if ((PressedState) == 0) {
			pObj->Widget.pEffect->pfDrawUp();  /* _WIDGET_EFFECT_3D_DrawUp(); */
			EffectSize = pObj->Widget.pEffect->EffectSize;
		}
		else {
			GUI_SetColor(RGB_BLACK);
			GUI_DrawRect(rClient.y0, rClient.x0, rClient.x1, rClient.y1);
			EffectSize = 1;
		}
		GUI__ReduceRect(&rInside, &rInside, EffectSize);
	}
#endif
	/* Draw background */
	GUI_SetBkColor(pObj->Props.aBkColor[ColorIndex]);
	GUI_SetColor(pObj->Props.aTextColor[ColorIndex]);
	WM_SetUserClipRect(&rInside);
	GUI_Clear();
	/* Draw bitmap.
	   If we have only one, we will use it.
	   If we have to we will use the second one (Index 1) for the pressed state
	*/
	if (ColorIndex < 2) {
		Index = (pObj->ahDrawObj[BUTTON_BI_PRESSED] && PressedState) ? BUTTON_BI_PRESSED : BUTTON_BI_UNPRESSED;
	}
	else {
		Index = pObj->ahDrawObj[BUTTON_BI_DISABLED] ? BUTTON_BI_DISABLED : BUTTON_BI_UNPRESSED;
	}
	GUI_DRAW__Draw(pObj->ahDrawObj[Index], 0, 0);
	/* Draw the actual button (background and text) */
	{
		GUI_RECT r;
		r = rInside;
#if BUTTON_USE_3D
		if (PressedState) {
			GUI_MoveRect(&r, BUTTON_3D_MOVE_X, BUTTON_3D_MOVE_Y);
		}
#endif
		GUI_SetTextMode(DRAWMODE_TRANS);
		GUI_DispStringInRect(s, &r, pObj->Props.Align);
	}
	/* Draw focus */
	if (State & BUTTON_STATE_FOCUS) {
		GUI_SetColor(RGB_BLACK);
		GUI_DrawFocusRect(&rClient, 2);
	}
	WM_SetUserClipRect(NULL);
}
/*********************************************************************
*
*       _Delete
*
* Delete attached objects (if any)
*/
static void _Delete(BUTTON_Obj *pObj) {
	GUI_ALLOC_FreePtr(&pObj->hpText);
	GUI_ALLOC_FreePtr(&pObj->ahDrawObj[0]);
	GUI_ALLOC_FreePtr(&pObj->ahDrawObj[1]);
}
static void _ButtonPressed(BUTTON_Obj *pObj) {
	WIDGET_OrState(pObj, BUTTON_STATE_PRESSED);
	if (pObj->Widget.Win.Status & WM_SF_ISVIS) {
		WM_NotifyParent(pObj, WM_NOTIFICATION_CLICKED);
	}
}
static void _ButtonReleased(BUTTON_Obj *pObj, int Notification) {
	WIDGET_AndState(pObj, BUTTON_STATE_PRESSED);
	if (pObj->Widget.Win.Status & WM_SF_ISVIS) {
		WM_NotifyParent(pObj, Notification);
	}
	if (Notification == WM_NOTIFICATION_RELEASED) {
		GUI_DEBUG_LOG("BUTTON: Hit\n");
		GUI_StoreKey(pObj->Widget.Id);
	}
}
static void _OnTouch(BUTTON_Obj *pObj, const GUI_PID_STATE *pState) {
#if BUTTON_REACT_ON_LEVEL
	if (!pState) /* Mouse moved out */
		_ButtonReleased(pObj, WM_NOTIFICATION_MOVED_OUT);
#else
	if (pState) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			if (!(pObj->Widget.State & BUTTON_STATE_PRESSED))
				_ButtonPressed(pObj);
		}
		/* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
		else if (pObj->Widget.State & BUTTON_STATE_PRESSED)
			_ButtonReleased(pObj, WM_NOTIFICATION_RELEASED);
	}
	else
		_ButtonReleased(pObj, WM_NOTIFICATION_MOVED_OUT);
#endif
}
static int _OnKey(BUTTON_Obj *pObj, const WM_KEY_INFO *pInfo) {
	switch (pInfo->Key) {
		case ' ':
			if (pInfo->PressedCnt > 0) /* Key pressed? */
				_ButtonPressed(pObj);
			else
				_ButtonReleased(pObj, WM_NOTIFICATION_RELEASED);
			return 1;
	}
	return 0;
}
#if BUTTON_REACT_ON_LEVEL
static void _OnPidStateChange(BUTTON_Obj *pObj, const WM_PID_STATE_CHANGED_INFO *pState) {
	if (pState->StatePrev == 0 && pState->State == 1) {
		if (!(pObj->Widget.State & BUTTON_STATE_PRESSED))
			_ButtonPressed(pObj);
	}
	else if (pState->StatePrev == 1 && pState->State == 0)
		if (pObj->Widget.State & BUTTON_STATE_PRESSED)
			_ButtonReleased(pObj, WM_NOTIFICATION_RELEASED);
}
#endif
WM_PARAM BUTTON_Callback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	BUTTON_Obj *pObj = hWin;
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	switch (MsgId) {
#if BUTTON_REACT_ON_LEVEL
		case WM_PID_STATE_CHANGED:
			_OnPidStateChange(pObj, (const WM_PID_STATE_CHANGED_INFO *)Data);
			return 0; /* Message handled. Do not call WM_DefaultProc, because the window may have been destroyed */
#endif
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)Data);
			return 0; /* Message handled. Do not call WM_DefaultProc, because the window may have been destroyed */
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_DELETE:
			GUI_DEBUG_LOG("BUTTON: _BUTTON_Callback(WM_DELETE)\n");
			_Delete(pObj);
			break; /* No return here ... WM_DefaultProc needs to be called */
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)Data))
				return 0;
			break;
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
BUTTON_Handle BUTTON_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id) {
	BUTTON_Handle hObj;
	GUI_USE_PARA(ExFlags);
	/* Create the window */
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, BUTTON_Callback,
								  sizeof(BUTTON_Obj) - sizeof(WM_Obj));
	if (hObj) {
		BUTTON_Obj *pObj = (hObj);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = BUTTON__DefaultProps;
	}
	else {
		GUI_DEBUG_ERROROUT_IF(hObj == 0, "BUTTON_Create failed")
	}

	return hObj;
}
void BUTTON_SetText(BUTTON_Handle hObj, const char *s) {
	if (hObj) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		if (GUI__SetText(&pObj->hpText, s)) {
			BUTTON_Invalidate(hObj);
		}

	}
}
void BUTTON_SetFont(BUTTON_Handle hObj, const GUI_FONT  *pfont) {
	if (hObj) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		pObj->Props.pFont = pfont;
		BUTTON_Invalidate(hObj);

	}
}
void BUTTON_SetBkColor(BUTTON_Handle hObj, unsigned int Index, RGB_COLOR Color) {
	if (hObj && (Index <= 2)) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		pObj->Props.aBkColor[Index] = Color;
		BUTTON_Invalidate(hObj);

	}
}
void BUTTON_SetTextColor(BUTTON_Handle hObj, unsigned int Index, RGB_COLOR Color) {
	if (hObj && (Index <= 2)) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		pObj->Props.aTextColor[Index] = Color;
		BUTTON_Invalidate(hObj);

	}
}
void BUTTON_SetState(BUTTON_Handle hObj, int State) {
	WIDGET_SetState(hObj, State);
}
void BUTTON_SetPressed(BUTTON_Handle hObj, int State) {
	if (State) {
		WIDGET_OrState(hObj, BUTTON_STATE_PRESSED);
	}
	else {
		WIDGET_AndState(hObj, BUTTON_STATE_PRESSED);
	}
}
void BUTTON_SetFocussable(BUTTON_Handle hObj, int State) {
	if (State) {
		WIDGET_OrState(hObj, WIDGET_STATE_FOCUSSABLE);
	}
	else {
		WIDGET_AndState(hObj, WIDGET_STATE_FOCUSSABLE);
	}
}

void BUTTON_SetBitmapEx(BUTTON_Handle hObj, unsigned int Index, const GUI_BITMAP *pBitmap, int x, int y) {
	BUTTON__SetDrawObj(hObj, Index, GUI_DRAW_BITMAP_Create(pBitmap, x, y));
}
void BUTTON_SetBitmap(BUTTON_Handle hObj, unsigned int Index, const GUI_BITMAP *pBitmap) {
	BUTTON_SetBitmapEx(hObj, Index, pBitmap, 0, 0);
}

BUTTON_Handle BUTTON_Create(int x0, int y0, int xsize, int ysize, int Id, int Flags) {
	return BUTTON_CreateEx(x0, y0, xsize, ysize, NULL, Flags, 0, Id);
}
BUTTON_Handle BUTTON_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags) {
	return BUTTON_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id);
}

BUTTON_Handle BUTTON_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	BUTTON_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = BUTTON_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
							pCreateInfo->xSize, pCreateInfo->ySize,
							hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	BUTTON_SetText(hThis, pCreateInfo->pName);
	return hThis;
}
void BUTTON_SetDefaultFont(const GUI_FONT  *pFont) {
	BUTTON__DefaultProps.pFont = pFont;
}
void BUTTON_SetDefaultTextColor(RGB_COLOR Color, unsigned Index) {
	if (Index < GUI_COUNTOF(BUTTON__DefaultProps.aTextColor)) {
		BUTTON__DefaultProps.aTextColor[Index] = Color;
	}
}
void BUTTON_SetDefaultBkColor(RGB_COLOR Color, unsigned Index) {
	if (Index < GUI_COUNTOF(BUTTON__DefaultProps.aBkColor)) {
		BUTTON__DefaultProps.aBkColor[Index] = Color;
	}
}
void BUTTON_SetDefaultTextAlign(int Align) {
	BUTTON__DefaultProps.Align = Align;
}
const GUI_FONT  *BUTTON_GetDefaultFont(void) {
	return BUTTON__DefaultProps.pFont;
}
RGB_COLOR BUTTON_GetDefaultTextColor(unsigned Index) {
	RGB_COLOR Color = GUI_INVALID_COLOR;
	if (Index < GUI_COUNTOF(BUTTON__DefaultProps.aTextColor)) {
		Color = BUTTON__DefaultProps.aTextColor[Index];
	}
	return Color;
}
RGB_COLOR BUTTON_GetDefaultBkColor(unsigned Index) {
	RGB_COLOR Color = GUI_INVALID_COLOR;
	if (Index < GUI_COUNTOF(BUTTON__DefaultProps.aBkColor)) {
		Color = BUTTON__DefaultProps.aBkColor[Index];
	}
	return Color;
}
int BUTTON_GetDefaultTextAlign(void) {
	return BUTTON__DefaultProps.Align;
}
RGB_COLOR BUTTON_GetBkColor(BUTTON_Handle hObj, unsigned int Index) {
	RGB_COLOR Color = 0;
	if (hObj && (Index < 2)) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		Color = pObj->Props.aBkColor[Index];

	}
	return Color;
}
const GUI_FONT  *BUTTON_GetFont(BUTTON_Handle hObj) {
	const GUI_FONT  *pFont = 0;
	if (hObj) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		pFont = pObj->Props.pFont;

	}
	return pFont;
}
void BUTTON_GetText(BUTTON_Handle hObj, char *pBuffer, int MaxLen) {
	if (hObj) {
		BUTTON_Obj *pObj;
		pObj = (hObj);
		if (pObj->hpText) {
			const char *pText = (const char *)(pObj->hpText);
			int Len = GUI__strlen(pText);
			if (Len > (MaxLen - 1))
				Len = MaxLen - 1;
			GUI__memcpy((void *)pBuffer, (const void *)pText, Len);
			*(pBuffer + Len) = 0;
		}
		else {
			*pBuffer = 0;     /* Empty string */
		}
	}
}
unsigned BUTTON_IsPressed(BUTTON_Handle hObj) {
	unsigned r = 0;
	if (hObj) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		r = (pObj->Widget.State & BUTTON_STATE_PRESSED) ? 1 : 0;

	}
	return r;
}

void BUTTON_SetSelfDrawEx(BUTTON_Handle hObj, unsigned int Index, GUI_DRAW_SELF_CB *pDraw, int x, int y) {
	BUTTON__SetDrawObj(hObj, Index, GUI_DRAW_SELF_Create(pDraw, x, y));
}
void BUTTON_SetSelfDraw(BUTTON_Handle hObj, unsigned int Index, GUI_DRAW_SELF_CB *pDraw) {
	BUTTON_SetSelfDrawEx(hObj, Index, pDraw, 0, 0);
}

void BUTTON_SetTextAlign(BUTTON_Handle hObj, int Align) {
	if (hObj) {
		BUTTON_Obj *pObj;

		pObj = (hObj);
		pObj->Props.Align = Align;
		BUTTON_Invalidate(hObj);

	}
}

void BUTTON__SetDrawObj(BUTTON_Handle hObj, int Index, GUI_DRAW_HANDLE hDrawObj) {
	if (hObj) {
		BUTTON_Obj *pObj;
		pObj = (hObj);
		if ((unsigned int)Index <= GUI_COUNTOF(pObj->ahDrawObj)) {
			GUI_ALLOC_FreePtr(&pObj->ahDrawObj[Index]);
			pObj->ahDrawObj[Index] = hDrawObj;
			BUTTON_Invalidate(hObj);
		}

	}
}
