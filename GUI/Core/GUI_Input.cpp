import TUX;
import TUX.Window;

#pragma region Key Message Handling
static KEY_STATE _KeyState{ 0 };
uint8_t _KeyStateCnt = 0;
void GUI_KEY_Store(const KEY_STATE &State) {
	_KeyState = State;
	_KeyStateCnt = 1;
}
bool GUI_PollKeyMsg(void) {
	if (!_KeyStateCnt) return false;
	_KeyStateCnt--;
	WObj::OnKey(_KeyState);
	return true;
}	
#pragma endregion

#pragma region Touch Input Handling
static PID_STATE _PidState{ 0 };
PID_STATE GUI_PID_Get(void) {
	return _PidState;
}
void GUI_PID_Store(const PID_STATE &State) {
	_PidState = State;
}
#pragma endregion
