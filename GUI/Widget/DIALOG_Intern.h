#pragma once

#include "GUIDebug.h"
#include "GUI_Protected.h"
#include "WM_Intern.h"

typedef struct  GUI_WIDGET_CREATE_INFO_struct GUI_WIDGET_CREATE_INFO;
typedef WM_Obj * GUI_WIDGET_CREATE_FUNC(const GUI_WIDGET_CREATE_INFO *pCreate, WM_Obj * hWin, int x0, int y0, WM_CALLBACK *cb);
struct GUI_WIDGET_CREATE_INFO_struct {
	GUI_WIDGET_CREATE_FUNC *pfCreateIndirect;
	const char *pName;                     /* Text ... Not used on all widgets */
	int16_t Id;                                /* ID ... should be unique in a dialog */
	int16_t x0, y0, xSize, ySize; /* Define position and size */
	uint16_t Flags; /* Widget specific create flags (opt.) */
	int32_t Para; /* Widget specific parameter (opt.) */
};

int     GUI_ExecDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_Obj * hParent, int x0, int y0);
int     GUI_ExecCreatedDialog(WM_Obj * hDialog);
WM_Obj * GUI_CreateDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_Obj * hParent, int x0, int y0);
void    GUI_SetDialogStatusPtr(WM_Obj * hDialog, WM_DIALOG_STATUS *pDialogStatus);
WM_DIALOG_STATUS *GUI_GetDialogStatusPtr(WM_Obj * hDialog);
void    GUI_EndDialog(WM_Obj * hWin, int r);
RGBC DIALOG_GetBkColor(void); /* obsolete */
RGBC DIALOG_SetBkColor(RGBC BkColor); /* obsolete */
