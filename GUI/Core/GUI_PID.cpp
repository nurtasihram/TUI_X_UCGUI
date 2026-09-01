import TUX;

static PID_STATE _State{ 0 };
PID_STATE GUI_PID_GetState(void) {
	return _State;
}
void GUI_PID_StoreState(const PID_STATE &State) {
	_State = State;
}
