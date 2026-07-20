#include <string.h>

#include "GUI.h"
#include "BUTTON.h"
#include "FRAMEWIN.h"
#include "FRAMEWIN_Private.h"
#include "TEXT.h"
#include "DIALOG.h"
#include "MESSAGEBOX.h"

#define MESSAGEBOX_BORDER 4
#define MESSAGEBOX_XSIZEOK 50
#define MESSAGEBOX_YSIZEOK 20
#define MESSAGEBOX_BKCOLOR RGB_WHITE

static void _OnKey(WM_HWIN hWin, const WM_KEY_INFO *pInfo) {
	int Key = pInfo->Key;
	if (pInfo->PressedCnt) {
		switch (Key) {
			case GUI_KEY_ESCAPE:
				GUI_EndDialog(hWin, 1); /* End dialog with return value 1 if <ESC> is pressed */
				break;
			case GUI_KEY_ENTER:
				GUI_EndDialog(hWin, 0); /* End dialog with return value 0 if <ENTER> is pressed */
				break;
		}
	}
}
static WM_PARAM _MESSAGEBOX_cbCallback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG:
			FRAMEWIN_SetClientColor(hWin, MESSAGEBOX_BKCOLOR);
			return 0;
		case WM_KEY:
			_OnKey(hWin, (const WM_KEY_INFO *)Data);
			return 0;
		case WM_NOTIFY_PARENT: {
			auto pInfo = (const WM_NOTIFY_INFO *)Data;
			auto hWinSrc = pInfo->hWinSrc;
			int Id = WM_GetId(hWinSrc); /* Get control ID */
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_RELEASED: /* React only if released */
					if (Id == GUI_ID_OK)
						GUI_EndDialog(hWin, 0); /* End dialog with return value 0 if OK */
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
WM_HWIN MESSAGEBOX_Create(const char *sMessage, const char *sCaption, int Flags) {
	GUI_WIDGET_CREATE_INFO _aDialogCreate[3];                                     /* 0: FrameWin, 1: Text, 2: Button */
	int BorderSize = FRAMEWIN_Obj::DefaultProps.BorderSize;                       /* Default border size of frame window */
	int xSizeFrame = MESSAGEBOX_XSIZEOK + 2 * BorderSize + MESSAGEBOX_BORDER * 2; /* XSize of frame window */
	int ySizeFrame;                                                               /* YSize of frame window */
	int x0, y0;                                                                   /* Position of frame window */
	int xSizeMessage;                                                             /* Length in pixels of message */
	int xSizeCaption;                                                             /* Length in pixels of caption */
	int ySizeCaption;                                                             /* YSize of caption */
	int ySizeMessage;                                                             /* YSize of message */
	GUI_RECT Rect;
	/* Zeroinit variables */
	memset(_aDialogCreate, 0, sizeof(_aDialogCreate));
	/* Get dimension of message */
	//auto pOldFont = GUI_SetFont(TEXT_GetDefaultFont());	/////////// FIX ///////////
	GUI_GetTextExtend(&Rect, sMessage, 255);
	xSizeMessage = Rect.x1 - Rect.x0 + MESSAGEBOX_BORDER * 2;
	ySizeMessage = Rect.y1 - Rect.y0 + 1;
	if (xSizeFrame < (xSizeMessage + 4 + MESSAGEBOX_BORDER * 2)) {
		xSizeFrame = xSizeMessage + 4 + MESSAGEBOX_BORDER * 2;
	}
	ySizeCaption = GUI_GetYSizeOfFont(FRAMEWIN_Obj::DefaultProps.pFont);
	ySizeFrame = ySizeMessage +            /* size of message */
		MESSAGEBOX_YSIZEOK +      /* size of button */
		ySizeCaption +            /* caption size */
		MESSAGEBOX_BORDER * 3 +   /* inner border - text, text - button, button - bottom */
		BorderSize * 2 +          /* top & bottom border */
		1;                        /* inner border */
	/* Get xsize of caption */
	xSizeCaption = GUI_GetStringDistX(sCaption);
	if (xSizeFrame < xSizeCaption + BorderSize * 2) {
		xSizeFrame = xSizeCaption + BorderSize * 2;
	}
	/* Check maximum */
	if (xSizeFrame > LCD_GetXSize()) {
		xSizeFrame = LCD_GetXSize();
	}
	if (ySizeFrame > LCD_GetYSize()) {
		ySizeFrame = LCD_GetYSize();
	}
	/* Calculate position of framewin */
	x0 = (LCD_GetXSize() - xSizeFrame) / 2;
	y0 = (LCD_GetYSize() - ySizeFrame) / 2;
	/* restore modified Context */
	//GUI_SetFont(pOldFont); /////////// FIX ///////////
	/* Fill frame win resource */
	_aDialogCreate[0].pfCreateIndirect = FRAMEWIN_CreateIndirect;
	_aDialogCreate[0].pName = sCaption;
	_aDialogCreate[0].x0 = x0;
	_aDialogCreate[0].y0 = y0;
	_aDialogCreate[0].xSize = xSizeFrame;
	_aDialogCreate[0].ySize = ySizeFrame;
	if (Flags & GUI_MESSAGEBOX_CF_MOVEABLE) {
		_aDialogCreate[0].Flags = FRAMEWIN_CF_MOVEABLE;
	}
	/* Fill text resource */
	_aDialogCreate[1].pfCreateIndirect = TEXT_CreateIndirect;
	_aDialogCreate[1].pName = sMessage;
	_aDialogCreate[1].x0 = (xSizeFrame - xSizeMessage - BorderSize * 2) / 2;
	_aDialogCreate[1].y0 = MESSAGEBOX_BORDER;
	_aDialogCreate[1].xSize = xSizeMessage;
	_aDialogCreate[1].ySize = ySizeMessage;
	_aDialogCreate[1].Para = TEXTALIGN_TOP | TEXTALIGN_HCENTER;
	/* Fill button resource */
	_aDialogCreate[2].pfCreateIndirect = BUTTON_CreateIndirect;
	_aDialogCreate[2].pName = "OK";
	_aDialogCreate[2].Id = GUI_ID_OK;
	_aDialogCreate[2].x0 = (xSizeFrame - MESSAGEBOX_XSIZEOK - BorderSize * 2) / 2;
	_aDialogCreate[2].y0 = MESSAGEBOX_BORDER * 2 + ySizeMessage;
	_aDialogCreate[2].xSize = MESSAGEBOX_XSIZEOK;
	_aDialogCreate[2].ySize = MESSAGEBOX_YSIZEOK;
	/* Create dialog */
	return GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _MESSAGEBOX_cbCallback, 0, 0, 0);
}
int GUI_MessageBox(const char *sMessage, const char *sCaption, int Flags) {
	WM_HWIN hWin;
	hWin = MESSAGEBOX_Create(sMessage, sCaption, Flags);
	/* Exec dialog */
	return GUI_ExecCreatedDialog(hWin);
}
