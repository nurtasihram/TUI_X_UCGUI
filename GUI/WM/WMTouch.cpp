#include "WM_Intern.h"
#include "GUIDebug.h"
WM_CRITICAL_HANDLE  WM__CHWinModal;
WM_CRITICAL_HANDLE  WM__CHWinLast;

static bool _IsInModalArea(WM_HWIN hWin) {
	return (!WM__CHWinModal.hWin ||
		WM__IsAncestor(hWin, WM__CHWinModal.hWin) ||
		WM__CHWinModal.hWin == hWin) ? true : false;
}

static void _SendMessageIfEnabled(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	if (WM_IsEnabled(hWin))
		WM__SendMessage(hWin, MsgId, Data);
}

static void _SendTouchMessage(WM_HWIN hWin, int MsgId, GUI_PID_STATE *pState) {
	if (pState) {
		WM_Obj *pWin = (WM_Obj *)hWin;
		pState->x -= pWin->Rect.x0;
		pState->y -= pWin->Rect.y0;
	}
	_SendMessageIfEnabled(hWin, MsgId, (WM_PARAM)pState);
	/* Send notification to all ancestors.
	   We need to check if the window which has received the last message still exists,
	   since it may have deleted itself and its parent as result of the message.
	*/
	while (WM_IsWindow(hWin)) {
		hWin = WM_GetParent(hWin);
		if (hWin)
			_SendMessageIfEnabled(hWin, WM_TOUCH_CHILD, (WM_PARAM)pState); /* Send message to the ancestors */
	}
}

static WM_HWIN _Screen2Win(GUI_PID_STATE *pState) {
	if (WM__hCapture)
		return WM__hCapture;
	return WM_Screen2hWin(pState->x, pState->y);
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
int WM_HandlePID(void) {
	int r = 0;
	WM_CRITICAL_HANDLE CHWin;
	GUI_PID_STATE StateNew;
	GUI_PID_GetState(&StateNew);
	WM__AddCriticalHandle(&CHWin);
	if ((WM_PID__StateLast.x != StateNew.x) || (WM_PID__StateLast.y != StateNew.y) || (WM_PID__StateLast.Pressed != StateNew.Pressed)) {
#if GUI_SUPPORT_CURSOR
		GUI_CURSOR_SetPosition(StateNew.x, StateNew.y);
#endif
		CHWin.hWin = _Screen2Win(&StateNew);
		if (_IsInModalArea(CHWin.hWin)) {
			/*
			 * Send WM_PID_STATE_CHANGED message if state has changed (just pressed or just released)
			 */
			if ((WM_PID__StateLast.Pressed != StateNew.Pressed) && CHWin.hWin) {
				WM_PID_STATE_CHANGED_INFO Info;
				WM_Obj *pWin = (WM_Obj *)CHWin.hWin;
				Info.State = StateNew.Pressed;
				Info.StatePrev = WM_PID__StateLast.Pressed;
				Info.x = StateNew.x - pWin->Rect.x0;
				Info.y = StateNew.y - pWin->Rect.y0;
				_SendMessageIfEnabled(CHWin.hWin, WM_PID_STATE_CHANGED, (WM_PARAM)&Info);
			}
			/*
			 * Send WM_TOUCH message(s)
			 * Note that we may have to send 2 touch messages.
			 */
			if (WM_PID__StateLast.Pressed | StateNew.Pressed) { /* Only if pressed or just released */
				r = 1;
				/*
				 * Tell window if it is no longer pressed
				 * This happens for 2 possible reasons:
				 * a) PID is released
				 * b) PID is moved out
				 */
				if (WM__CHWinLast.hWin != CHWin.hWin) {
					if (WM__CHWinLast.hWin) {
						GUI_DEBUG_LOG("\nSending WM_Touch to LastWindow %d (out of area)", WM__CHWinLast.hWin);
						GUI_PID_STATE *pState = StateNew.Pressed ? NULL : &WM_PID__StateLast;
						_SendTouchMessage(WM__CHWinLast.hWin, WM_TOUCH, pState);
						WM__CHWinLast.hWin = NULL;
					}
				}
				/* Sending WM_Touch to current window */
				if (CHWin.hWin) {
					/* Remember window */
					if (StateNew.Pressed)
						WM__CHWinLast.hWin = CHWin.hWin;
					else {
						/* Handle automatic capture release */
						if (WM__CaptureReleaseAuto)
							WM_ReleaseCapture();
						WM__CHWinLast.hWin = NULL;
					}
					_SendTouchMessage(CHWin.hWin, WM_TOUCH, &StateNew);
				}
			}
#if GUI_SUPPORT_MOUSE
			/* Send WM_MOUSEOVER Message */
			else if (CHWin.hWin)
				/* Do not send messages to disabled windows */
				if (WM_IsEnabled(CHWin.hWin))
					_SendTouchMessage(CHWin.hWin, WM_MOUSEOVER, &StateNew);
#endif
		}
		/* Store the new state */
		GUI_PID_GetState(&StateNew);
		WM_PID__StateLast = StateNew;
	}
	WM__RemoveCriticalHandle(&CHWin);

	return r;
}
