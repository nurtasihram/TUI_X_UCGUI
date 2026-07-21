#pragma once

#include "WM.h"

typedef struct  GUI_WIDGET_CREATE_INFO_struct GUI_WIDGET_CREATE_INFO;
typedef WM_Obj * GUI_WIDGET_CREATE_FUNC(const GUI_WIDGET_CREATE_INFO *pCreate, WM_Obj * hWin, int x0, int y0, WM_CALLBACK *cb);
struct GUI_WIDGET_CREATE_INFO_struct {
	GUI_WIDGET_CREATE_FUNC *pfCreateIndirect;
	const char *pName;                     /* Text ... Not used on all widgets */
	int16_t Id;                                /* ID ... should be unique in a dialog */
	int16_t x0, y0, xSize, ySize;              /* Define position and size */
	uint16_t Flags;                             /* Widget specific create flags (opt.) */
	int32_t Para;                              /* Widget specific parameter (opt.) */
};

WM_Obj * BUTTON_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_Obj * CHECKBOX_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_Obj * EDIT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_Obj * FRAMEWIN_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_Obj * RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_Obj * SLIDER_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_Obj * SCROLLBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_Obj * TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
int     GUI_ExecDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_Obj * hParent, int x0, int y0);
int     GUI_ExecCreatedDialog(WM_Obj * hDialog);
WM_Obj * GUI_CreateDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_Obj * hParent, int x0, int y0);
void    GUI_SetDialogStatusPtr(WM_Obj * hDialog, WM_DIALOG_STATUS *pDialogStatus); /* not to documented */
WM_DIALOG_STATUS *GUI_GetDialogStatusPtr(WM_Obj * hDialog);                       /* not to documented */
void    GUI_EndDialog(WM_Obj * hWin, int r);
RGBC DIALOG_GetBkColor(void);                                                /* obsolete */
RGBC DIALOG_SetBkColor(RGBC BkColor);                                   /* obsolete */
