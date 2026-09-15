

#include "GUI.h"

#include "WM.h"

#pragma region Key Message Handling
static int _KeyMsgCnt;
static struct {
	int Key;
	int PressedCnt;
} _KeyMsg;

static int _Key;
int GUI_GetKey(void) {
	int r = _Key;
	_Key = 0;
	return r;
}
void GUI_StoreKey(int Key) {
	if (!_Key) {
		_Key = Key;
	}
}
void GUI_ClearKeyBuffer(void) {
	while (GUI_GetKey());
}
void GUI_StoreKeyMsg(int Key, int PressedCnt) {
	_KeyMsg.Key = Key;
	_KeyMsg.PressedCnt = PressedCnt;
	_KeyMsgCnt = 1;
}
bool GUI_PollKeyMsg(void) {
	if (!_KeyMsgCnt) return false;
	_KeyMsgCnt--;
	auto Key = _KeyMsg.Key;
	WM_OnKey(Key, _KeyMsg.PressedCnt);
	if (_KeyMsg.PressedCnt == 1)
		GUI_StoreKey(Key);
	return true;
}
/*********************************************************************
*
*       GUI_SendKeyMsg
*
* Purpose:
*   Send the key to a window using the window manager (if available).
*   If no window is ready to take the input, we call the store routine
*   and wait for somebody to poll the buffer.
*/
void GUI_SendKeyMsg(int Key, int PressedCnt) {
	if (!WM_OnKey(Key, PressedCnt))
		GUI_StoreKeyMsg(Key, PressedCnt);
}
#pragma endregion

#pragma region Touch Input Handling
static PID_STATE _State{ 0 };
PID_STATE GUI_PID_GetState(void) {
	return _State;
}
void GUI_PID_StoreState(const PID_STATE &State) {
	_State = State;
}
#pragma endregion
