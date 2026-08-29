#include "WM_Intern.h"
#include "GUI_Protected.h"
#include "GUIDebug.h"

PID_STATE WM_PID__StateLast{ 0 };
PID_STATE WM_PID__GetPrevState() {
	return WM_PID__StateLast;
}

#pragma region Mouse/Touch
static void _SendMessageIfEnabled(WObj *pWin, int MsgId, WM_PARAM Data) {
	if (pWin->IsEnabled())
		WM__SendMessage(pWin, MsgId, Data);
}

static void _SendTouchMessage(WObj *pWin, int MsgId, PID_STATE *pState) {
	if (pState)
		*pState -= pWin->Rect.LeftTop();
	_SendMessageIfEnabled(pWin, MsgId, (WM_PARAM)pState);
	/* Send notification to all ancestors.
	   We need to check if the window which has received the last message still exists,
	   since it may have deleted itself and its parent as result of the message.
	*/
	while (WObj::IsWindow(pWin)) {
		pWin = pWin->Parent();
		if (pWin)
			_SendMessageIfEnabled(pWin, WM_TOUCH_CHILD, (WM_PARAM)pState); /* Send message to the ancestors */
	}
}

/*********************************************************************
*
*       WM_HandlePID
*
* Polls the touch screen. If something has changed,
* sends a message to the concerned window.
*
* Return value:
*   0 if nothing has been done
*   1 if touch message has been sent
*/
bool WM_HandlePID(void) {
	auto StateNew = GUI_PID_GetState();
	if (WM_PID__StateLast == StateNew) return false;
	bool r = false;
#if GUI_SUPPORT_CURSOR
	GUI_CURSOR_SetPosition(StateNew.x, StateNew.y);
#endif
	WObj::CriticalHandle CHWin = WObj::pWinCapture ? WObj::pWinCapture : WM_Screen2Win(StateNew);
	CHWin.Add();
	/* Send WM_PID_STATE_CHANGED message if state has changed (just pressed or just released) */
	if (WM_PID__StateLast.Pressed != StateNew.Pressed && CHWin.pWin) {
		PID_CHANGED_INFO Info;
		auto pWin = CHWin.pWin;
		Info.State = StateNew.Pressed;
		Info.StatePrev = WM_PID__StateLast.Pressed;
		Info.x = StateNew.x - pWin->Rect.x0;
		Info.y = StateNew.y - pWin->Rect.y0;
		_SendMessageIfEnabled((WObj *)CHWin.pWin, WM_PID_STATE_CHANGED, (WM_PARAM)&Info);
	}
	/* Send WM_TOUCH message(s) Note that we may have to send 2 touch messages. */
	if (WM_PID__StateLast.Pressed | StateNew.Pressed) { /* Only if pressed or just released */
		r = 1;
		/* Tell window if it is no longer pressed
		* This happens for 2 possible reasons:
		* a) PID is released
		* b) PID is moved out
		*/
		if (WObj::CHWinLast.pWin != CHWin.pWin) {
			if (WObj::CHWinLast.pWin) {
				GUI_DEBUG_LOG("\nSending WM_TOUCH to LastWindow %d (out of area)", WObj::CHWinLast.pWin);
				PID_STATE *pState = StateNew.Pressed ? nullptr : &WM_PID__StateLast;
				_SendTouchMessage((WObj *)WObj::CHWinLast.pWin, WM_TOUCH, pState);
				WObj::CHWinLast.pWin = nullptr;
			}
		}
		/* Sending WM_TOUCH to current window */
		if (CHWin.pWin) {
			/* Remember window */
			if (StateNew.Pressed)
				WObj::CHWinLast.pWin = CHWin.pWin;
			else {
				/* Handle automatic capture release */
				if (WObj::WM__CaptureReleaseAuto)
					WObj::ReleaseCapture();
				WObj::CHWinLast.pWin = nullptr;
			}
			_SendTouchMessage((WObj *)CHWin.pWin, WM_TOUCH, &StateNew);
		}
	}
#if GUI_SUPPORT_MOUSE
	/* Send WM_MOUSEOVER Message */
	else if (CHWin.pWin)
		/* Do not send messages to disabled windows */
		if (CHWin.pWin->IsEnabled())
			_SendTouchMessage((WObj *)CHWin.pWin, WM_MOUSEOVER, &StateNew);
#endif
	CHWin.Remove();
	/* Store the new state */
	WM_PID__StateLast = GUI_PID_GetState();
	return r;
}
#pragma endregion

bool WM_Exec1(void) {
	/* Poll PID if necessary */
	if (WM_HandlePID())
		return true; /* We have done something ... */
	if (GUI_PollKeyMsg())
		return true; /* We have done something ... */
	if (WObj::DrawOnce())
		return true; /* We have done something ... */
	return false; /* There was nothing to do ... */
}
bool WM_Exec(void) {
	bool r = false;
	while (WM_Exec1())
		r = true; /* We have done something */
	return r;
}

WM_PARAM WM__SendMessage(WObj * pWin, int MsgId, WM_PARAM Data) {
	if (pWin->cb)
		return pWin->cb(pWin, MsgId, Data);
	return WM_DefaultProc(pWin, MsgId, Data);
}

/*********************************************************************
*
*       WM_DefaultProc
*
* Purpose
*   Default callback for windows
*   Any window should call this routine in the "default" part of the
*   its callback function for messages it does not handle itself.
*
*/
WM_PARAM WM_DefaultProc(WObj * pWin, int MsgId, WM_PARAM Data) {
	/* Exec message */
	switch (MsgId) {
		case WM_GET_INSIDE_RECT: /* return client window in absolute (screen) coordinates */
			*(RECT *)Data = pWin->GetClientRect();
			return 0;
		case WM_GET_CLIENT_WINDOW: /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			return (WM_PARAM)pWin;
		case WM_KEY: 
			pWin->Parent()->Require(WM_KEY, Data);
			return 0;	
		case WM_GET_BKCOLOR:
			return RGB_INVALID_COLOR;
		case WM_NOTIFY_ENABLE:
			pWin->Invalidate();
			return 0;
	}
	/* Message not handled. If it queries something, we return 0 to be on the safe side. */
	return 0;
}

RECT WM_GetClientRect() {
	return WObj::pWinActive->GetClientRect();
}
RECT WM_GetInsideRect() {
	return WObj::pWinActive->GetInsideRect();
}

int WM_OnKey(int Key, int Pressed) {
	if (WObj::pWinFocus) {
		WM_KEY_INFO Info;
		Info.Key = Key;
		Info.PressedCnt = Pressed;
		WM__SendMessage(WObj::pWinFocus, WM_KEY, (WM_PARAM)&Info);
		return 1;
	}
	return 0;
}

void WM_Init(void) {
	static bool _IsInited = false;
	if (_IsInited)
		return;
	GUI.WM__pUserClipRect = nullptr;
	/* Make sure we have at least one window. This greatly simplifies the
		drawing routines as they do not have to check if the window is valid.
	*/
	WObj::pWinDesktop = new WObj({ 0, 0, GUI_XMAX, GUI_YMAX }, WC_VISIBLE, WObj::cbBackWin);
	WObj::pWinDesktop->Invalidate(); /* Required because a desktop window has no parent. */
	/* Register the critical handles ... Note: This could be moved into the module setting the Window handle */
	WObj::CHWinLast.Add();
	WObj::pWinDesktop->Select();
	WObj::Activate();
	_IsInited = true;
}
