module;

#include "WM_Intern.h"

export module TUX.Widget.Window;

import TUX.Widget;

#define WINDOW_BKCOLOR_DEFAULT RGB_GRAYL(0xC0)

export {

class Window : public Widget {

public:

	struct Properties {
		RGBC BkColor{ WINDOW_BKCOLOR_DEFAULT };
	} static DefaultProps;

private:

	Properties Props = DefaultProps;
	WM_CALLBACK *cb;
	WObj *pFocussedChild = nullptr;
	DIALOG_STATUS *pDialogStatus = nullptr;

private:

	void _OnChildHasFocus(const NOTIFY_CHILD_HAS_FOCUS_INFO* pInfo) {
		if (pInfo)
			if (!WM__IsAncestorOrSelf(pInfo->pNew, this)) /* A child has received the focus, Framewindow needs to be activated */
				/* Remember the child which had the focus so we can reactive this child */
				if (WM__IsAncestor(pInfo->pOld, this))
					pFocussedChild = pInfo->pOld;
	}
	void _OnKey(const WM_KEY_INFO* pInfo) {
		if (pInfo->PressedCnt > 0) {
			switch (pInfo->Key) {
			case GUI_KEY_TAB:
				pFocussedChild = WM_SetFocusOnNextChild(this);
				break; /* Send to parent by not doing anything */
			}
		}
	}
	static WM_PARAM _cb(WObj* pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Window*)pWin;
		auto cb = pObj->cb;
		switch (MsgId) {
		case WM_HANDLE_DIALOG_STATUS:
			if (Data) /* set pointer to Dialog status */
				pObj->pDialogStatus = (DIALOG_STATUS*)Data;
			return (WM_PARAM)pObj->pDialogStatus;
		case WM_SET_FOCUS:
			if (Data) { /* Focus received */
				if (pObj->pFocussedChild && pObj->pFocussedChild != pObj)
					pObj->pFocussedChild->SetFocus();
				else
					pObj->pFocussedChild = WM_SetFocusOnNextChild(pObj);
			}
			return 0;
		case WM_GET_ACCEPT_FOCUS:
			pObj->HandleActive(MsgId, &Data);
			return Data;
		case WM_NOTIFY_CHILD_HAS_FOCUS:
			pObj->_OnChildHasFocus((const NOTIFY_CHILD_HAS_FOCUS_INFO*)Data);
			return 0;
		case WM_KEY:
			pObj->_OnKey((const WM_KEY_INFO*)Data);
			break;
		case WM_PAINT:
			GUI.SetBkColor(pObj->Props.BkColor);
			GUI_Clear();
			return 0;
		case WM_GET_BKCOLOR:
			return pObj->Props.BkColor;
		}
		if (cb)
			return cb(pWin, MsgId, Data);
		return WM_DefaultProc(pWin, MsgId, Data);
	}
public:
	Window(RECT r, WM_CF Style, WObj *pParent, uint16_t Id, WM_CALLBACK *cb) :
		Widget(r, Style, _cb, pParent, Id, WIDGET_STATE_FOCUSSABLE),
		cb(cb) {}
	static WObj* CreateIndirect(const CreateStruct* pCreateInfo, WObj* hWinParent, int x0, int y0, WM_CALLBACK* cb) {
		return new Window(
			RECT(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
				 pCreateInfo->x0 + x0 + pCreateInfo->xSize - 1,
				 pCreateInfo->y0 + y0 + pCreateInfo->ySize - 1),
			pCreateInfo->Flags, hWinParent, pCreateInfo->Id,
			cb);
	}
};

}

Window::Properties Window::DefaultProps;
