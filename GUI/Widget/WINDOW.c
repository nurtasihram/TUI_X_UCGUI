

#include "DIALOG.h"

#define WINDOW_BKCOLOR_DEFAULT RGB_GRAYL(0xC0)

typedef struct {
	WIDGET Widget;
	WM_CALLBACK *cb;
	WM_HWIN hFocussedChild;
	WM_DIALOG_STATUS *pDialogStatus;
} WINDOW_OBJ;
RGB_COLOR WINDOW__DefaultBkColor = WINDOW_BKCOLOR_DEFAULT;
static void _OnChildHasFocus(WINDOW_OBJ *pObj, const WM_MESSAGE *pMsg) {
	if (pMsg->Data) {
		const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *pInfo = (const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *)pMsg->Data;
		int IsDesc = WM__IsAncestorOrSelf(pInfo->hNew, pObj);
		if (!IsDesc) {  /* A child has received the focus, Framewindow needs to be activated */
			/* Remember the child which had the focus so we can reactive this child */
			if (WM__IsAncestor(pInfo->hOld, pObj)) {
				pObj->hFocussedChild = pInfo->hOld;
			}
		}
	}
}
static void _cb(WM_MESSAGE *pMsg) {
	WM_HWIN hObj;
	WINDOW_OBJ *pObj;
	WM_CALLBACK *cb;
	hObj = pMsg->hWin;
	pObj = (hObj);
	cb = pObj->cb;
	switch (pMsg->MsgId) {
		case WM_HANDLE_DIALOG_STATUS:
			if (pMsg->Data) { /* set pointer to Dialog status */
				pObj->pDialogStatus = (WM_DIALOG_STATUS *)pMsg->Data;
			}
			else { /* return pointer to Dialog status */
				pMsg->Data = (WM_PARAM)pObj->pDialogStatus;
			}
			return;
		case WM_SET_FOCUS:
			if ((int)pMsg->Data) {   /* Focus received */
				if (pObj->hFocussedChild && (pObj->hFocussedChild != hObj)) {
					WM_SetFocus(pObj->hFocussedChild);
				}
				else {
					pObj->hFocussedChild = WM_SetFocusOnNextChild(hObj);
				}
				pMsg->Data = (WM_PARAM)0;   /* Focus change accepted */
			}
			return;
		case WM_GET_ACCEPT_FOCUS:
			WIDGET_HandleActive(hObj, pMsg);
			return;
		case WM_NOTIFY_CHILD_HAS_FOCUS:
			_OnChildHasFocus(pObj, pMsg);
			return;
		case WM_KEY:
			if (((const WM_KEY_INFO *)(pMsg->Data))->PressedCnt > 0) {
				int Key = ((const WM_KEY_INFO *)(pMsg->Data))->Key;
				switch (Key) {
					case GUI_KEY_TAB:
						pObj->hFocussedChild = WM_SetFocusOnNextChild(hObj);
						break;                    /* Send to parent by not doing anything */
				}
			}
			break;
		case WM_PAINT:
			GUI_SetBkColor(WINDOW__DefaultBkColor);
			GUI_Clear();
			break;
		case WM_GET_BKCOLOR:
			pMsg->Data = (WM_PARAM)(uintptr_t)WINDOW__DefaultBkColor;
			return;                       /* Message handled */
	}
	if (cb) {
		(*cb)(pMsg);
	}
	else {
		WM_DefaultProc(pMsg);
	}
}
WM_HWIN WINDOW_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	WM_HWIN hObj;
	hObj = WM_CreateWindowAsChild(
		pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize, hWinParent,
		pCreateInfo->Flags, _cb, sizeof(WINDOW_OBJ) - sizeof(WM_Obj));
	if (hObj) {
		WINDOW_OBJ *pObj;

		pObj = (hObj);
		WIDGET__Init(&pObj->Widget, pCreateInfo->Id, WIDGET_STATE_FOCUSSABLE);
		pObj->cb = cb;
		pObj->hFocussedChild = 0;

	}
	return hObj;
}

void WINDOW_SetDefaultBkColor(RGB_COLOR Color) {
	WINDOW__DefaultBkColor = Color;
}
