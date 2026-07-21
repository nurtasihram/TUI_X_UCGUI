#include "DIALOG.h"

#define WINDOW_BKCOLOR_DEFAULT RGB_GRAYL(0xC0)

struct WINDOW_Obj : public WIDGET {
	WM_CALLBACK *cb;
	WM_Obj * hFocussedChild;
	WM_DIALOG_STATUS *pDialogStatus;
};

auto WINDOW__DefaultBkColor = WINDOW_BKCOLOR_DEFAULT;

static void _OnChildHasFocus(WINDOW_Obj *pObj, const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *pInfo) {
	if (pInfo)
		if (!WM__IsAncestorOrSelf(pInfo->pNew, pObj)) /* A child has received the focus, Framewindow needs to be activated */
			/* Remember the child which had the focus so we can reactive this child */
			if (WM__IsAncestor(pInfo->pOld, pObj))
				pObj->hFocussedChild = pInfo->pOld;
}
static void _OnKey(WINDOW_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) {
		switch (pInfo->Key) {
			case GUI_KEY_TAB:
				pObj->hFocussedChild = WM_SetFocusOnNextChild(pObj);
				break; /* Send to parent by not doing anything */
		}
	}
}
static WM_PARAM _cb(WM_Obj * hWin, int MsgId, WM_PARAM Data) {
	auto pObj = (WINDOW_Obj *)hWin;
	auto cb = pObj->cb;
	switch (MsgId) {
		case WM_HANDLE_DIALOG_STATUS:
			if (Data) /* set pointer to Dialog status */
				pObj->pDialogStatus = (WM_DIALOG_STATUS *)Data;
			return (WM_PARAM)pObj->pDialogStatus;
		case WM_SET_FOCUS:
			if (Data) { /* Focus received */
				if (pObj->hFocussedChild && pObj->hFocussedChild != pObj)
					WM_SetFocus(pObj->hFocussedChild);
				else
					pObj->hFocussedChild = WM_SetFocusOnNextChild(pObj);
			}
			return 0;
		case WM_GET_ACCEPT_FOCUS:
			WIDGET_HandleActive(pObj, MsgId, &Data);
			return Data;
		case WM_NOTIFY_CHILD_HAS_FOCUS:
			_OnChildHasFocus(pObj, (const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *)Data);
			return 0;
		case WM_KEY:
			_OnKey(pObj, (const WM_KEY_INFO *)Data);
			break;
		case WM_PAINT:
			GUI_SetBkColor(WINDOW__DefaultBkColor);
			GUI_Clear();
			return 0;
		case WM_GET_BKCOLOR:
			return WINDOW__DefaultBkColor;
	}
	if (cb)
		return cb(hWin, MsgId, Data);
	return WM_DefaultProc(hWin, MsgId, Data);
}
WM_Obj * WINDOW_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	WM_Obj * hObj;
	hObj = WM_CreateWindowAsChild(
		pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize, hWinParent,
		pCreateInfo->Flags, _cb, sizeof(WINDOW_Obj) - sizeof(WM_Obj));
	if (hObj) {
		auto pObj = (WINDOW_Obj *)hObj;
		WIDGET__Init(pObj, pCreateInfo->Id, WIDGET_STATE_FOCUSSABLE);
		pObj->cb = cb;
		pObj->hFocussedChild = 0;
	}
	return hObj;
}
