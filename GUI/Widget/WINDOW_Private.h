#pragma once

#include "WM.h"
#include "WIDGET.h"

#define WINDOW_BKCOLOR_DEFAULT RGB_GRAYL(0xC0)

struct WINDOW_Obj : public WIDGET {
	struct Properties {
		RGBC BkColor{ WINDOW_BKCOLOR_DEFAULT };
	} static DefaultProps;
	Properties Props;
	WM_CALLBACK *cb;
	WM_Obj *hFocussedChild;
	WM_DIALOG_STATUS *pDialogStatus;
};
