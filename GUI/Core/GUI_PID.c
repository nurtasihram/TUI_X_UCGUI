#include "LCD_Private.h"      /* private modul definitions & config */
#include "GUI_Protected.h"

#include "WM.h"

static GUI_PID_STATE _State;

int GUI_PID_GetState(GUI_PID_STATE *pState) {
	*pState = _State;
	return (pState->Pressed != 0) ? 1 : 0;
}

void GUI_PID_StoreState(const GUI_PID_STATE *pState) {
	_State = *pState;
}
