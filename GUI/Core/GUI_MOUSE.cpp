#include "LCD_Private.h"      /* private modul definitions & config */
#include "GUI_Protected.h"

static GUI_PID_STATE _State;

int GUI_MOUSE_GetState(GUI_PID_STATE *pState) {
	*pState = _State;
	return (_State.Pressed != 0) ? 1 : 0;
}

void GUI_MOUSE_StoreState(const GUI_PID_STATE *pState) {
	_State = *pState;
	GUI_PID_StoreState(pState);
}
