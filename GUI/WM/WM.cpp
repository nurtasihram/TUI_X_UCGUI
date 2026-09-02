#include "GUI.h"

import TUX.Window;

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
		WObj::pWinFocus->Require(WM_KEY, (WM_PARAM)&Info);
		return 1;
	}
	return 0;
}

bool WM_Exec1(void) {
	/* Poll PID if necessary */
	if (WObj::HandlePID())
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

void WM_Init(void) {
	static bool _IsInited = false;
	if (_IsInited)
		return;
	GUI.WM__pUserClipRect = nullptr;
	/* Register the critical handles ... Note: This could be moved into the module setting the Window handle */
	WObj::CHWinLast.Add();
	/* Make sure we have at least one window. This greatly simplifies the
		drawing routines as they do not have to check if the window is valid.
	*/
	WObj::CreateDesktopWindow(RGB_INVALID);
	WObj::Activate();
	_IsInited = true;
}
