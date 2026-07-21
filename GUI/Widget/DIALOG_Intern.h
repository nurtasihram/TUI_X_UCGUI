#pragma once

#include "WM.h"

typedef struct  GUI_WIDGET_CREATE_INFO_struct GUI_WIDGET_CREATE_INFO;
typedef WM_HWIN GUI_WIDGET_CREATE_FUNC(const GUI_WIDGET_CREATE_INFO *pCreate, WM_HWIN hWin, int x0, int y0, WM_CALLBACK *cb);
struct GUI_WIDGET_CREATE_INFO_struct {
	GUI_WIDGET_CREATE_FUNC *pfCreateIndirect;
	const char *pName;                     /* Text ... Not used on all widgets */
	int16_t Id;                                /* ID ... should be unique in a dialog */
	int16_t x0, y0, xSize, ySize;              /* Define position and size */
	uint16_t Flags;                             /* Widget specific create flags (opt.) */
	int32_t Para;                              /* Widget specific parameter (opt.) */
};

WM_HWIN BUTTON_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_HWIN CHECKBOX_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_HWIN EDIT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_HWIN FRAMEWIN_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_HWIN RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_HWIN SLIDER_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_HWIN SCROLLBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
WM_HWIN TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb);
int     GUI_ExecDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_HWIN hParent, int x0, int y0);
int     GUI_ExecCreatedDialog(WM_HWIN hDialog);
WM_HWIN GUI_CreateDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_HWIN hParent, int x0, int y0);
void    GUI_SetDialogStatusPtr(WM_HWIN hDialog, WM_DIALOG_STATUS *pDialogStatus); /* not to documented */
WM_DIALOG_STATUS *GUI_GetDialogStatusPtr(WM_HWIN hDialog);                       /* not to documented */
void    GUI_EndDialog(WM_HWIN hWin, int r);
RGBC DIALOG_GetBkColor(void);                                                /* obsolete */
RGBC DIALOG_SetBkColor(RGBC BkColor);                                   /* obsolete */
