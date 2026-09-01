module;

#include "WM.h"

export module TUX.Widget.Window;

import TUX.Widget;

export {

class Window : public WObj {

public:

	struct Properties {
		RGBC BkColor{ RGB_GRAYL(0xE4) };
	} static DefaultProps;

private:

	Properties Props = DefaultProps;
	WM_CALLBACK *cb;
	WObj *pFocussedChild = nullptr;
	DIALOG_STATUS *pDialogStatus = nullptr;

private:

	void _OnChildHasFocus(const NOTIFY_CHILD_HAS_FOCUS_INFO* pInfo) {
		if (!pInfo) return;
		/* A child has received the focus, Framewindow needs to be activated */
		if (IsAncestorOrSelf(pInfo->pNew)) 
			pFocussedChild = pInfo->pNew;
		else if (IsAncestorOf(pInfo->pOld))
			/* Remember the child which had the focus so we can reactive this child */
			pFocussedChild = pInfo->pOld;
	}
	void _OnKey(const WM_KEY_INFO* pInfo) {
		if (pInfo->PressedCnt > 0) {
			switch (pInfo->Key) {
			case GUI_KEY_TAB:
				pFocussedChild = SetFocusOnNextChild();
				break; /* Send to parent by not doing anything */
			}
		}
	}
	static WM_PARAM _cb(WObj* pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Window*)pWin;
		auto cb = pObj->cb;
		switch (MsgId) {
		case WM_CREATE:
			return 0;
		case WM_PID_STATE_CHANGED:
			if (auto pInfo = (const PID_CHANGED_INFO *)Data)
				if (pInfo->State)
					pObj->SetFocus();
			break;
		case WM_HANDLE_DIALOG_STATUS:
			if (Data) /* set pointer to Dialog status */
				pObj->pDialogStatus = (DIALOG_STATUS*)Data;
			return (WM_PARAM)pObj->pDialogStatus;
		case WM_SET_FOCUS:
			if (Data) { /* Focus received */
				if (IsWindow(pObj->pFocussedChild) && pObj->pFocussedChild != pObj)
					pObj->pFocussedChild->SetFocus();
				else
					pObj->pFocussedChild = pObj->SetFocusOnNextChild();
			}
			return 0;
		case WM_GET_ACCEPT_FOCUS:
			return false;
		case WM_NOTIFY_CHILD_HAS_FOCUS:
			pObj->_OnChildHasFocus((const NOTIFY_CHILD_HAS_FOCUS_INFO*)Data);
			return 0;
		case WM_KEY:
			pObj->_OnKey((const WM_KEY_INFO*)Data);
			break;
		case WM_PAINT:
			GUI.BkColor(pObj->Props.BkColor);
			GUI_Clear();
			return 0;
		case WM_GET_BKCOLOR:
			return pObj->Props.BkColor;
		}
		if (cb)
			return cb(pWin, MsgId, Data);
		return DefaultProc(pWin, MsgId, Data);
	}
public:
	Window(RECT r, WM_CF Style, WObj *pParent, uint16_t Id, WM_CALLBACK *cb) :
		WObj(r, Style, _cb, pParent),
		cb(cb) {}
public:
	auto FocussedChild() const { return pFocussedChild; }
};

}

Window::Properties Window::DefaultProps;
