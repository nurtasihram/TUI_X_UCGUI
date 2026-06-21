
#include <stddef.h>           /* needed for definition of NULL */
#include "DIALOG.h"
#include "WIDGET.h"
#include "WM_Intern.h"

/* Define colors */
#ifndef DIALOG_BKCOLOR0_DEFAULT
#define DIALOG_BKCOLOR0_DEFAULT 0xc0c0c0
#endif
static LCD_COLOR _BkColor = DIALOG_BKCOLOR0_DEFAULT;
LCD_COLOR DIALOG_GetBkColor(void) {
	return _BkColor;
}
LCD_COLOR DIALOG_SetBkColor(LCD_COLOR BkColor) {
	LCD_COLOR r;
	r = _BkColor;
	_BkColor = BkColor;
	return r;
}
void GUI_SetDialogStatusPtr(WM_HWIN hDialog, WM_DIALOG_STATUS *pDialogStatus) {
	WM_MESSAGE Msg = { 0 };
	Msg.MsgId = WM_HANDLE_DIALOG_STATUS;
	Msg.Data.p = pDialogStatus;
	WM_SendMessage(hDialog, &Msg);
}
WM_DIALOG_STATUS *GUI_GetDialogStatusPtr(WM_HWIN hDialog) {
	WM_MESSAGE Msg = { 0 };
	Msg.MsgId = WM_HANDLE_DIALOG_STATUS;
	WM_SendMessage(hDialog, &Msg);
	return (WM_DIALOG_STATUS *)Msg.Data.p;
}
WM_HWIN GUI_CreateDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_HWIN hParent,
							int x0, int y0) {
	WM_HWIN hDialog = paWidget->pfCreateIndirect(paWidget, hParent, x0, y0, cb);     /* Create parent window */
	WM_HWIN hDialogClient = WM_GetClientWindow(hDialog);
	WIDGET_OrState(hDialog, paWidget->Flags);
	WM_ShowWindow(hDialog);
	WM_ShowWindow(hDialogClient);
	while (--NumWidgets > 0) {
		WM_HWIN hChild;
		paWidget++;
		hChild = paWidget->pfCreateIndirect(paWidget, hDialogClient, 0, 0, 0);     /* Create child window */
		WM_ShowWindow(hChild);
	}
	WM_SetFocusOnNextChild(hDialog);     /* Set the focus to the first child */
	WM_SendMessageNoPara(hDialogClient, WM_INIT_DIALOG);
	return hDialog;
}
void GUI_EndDialog(WM_HWIN hDialog, int r) {
	WM_DIALOG_STATUS *pStatus;
	pStatus = GUI_GetDialogStatusPtr(hDialog);
	if (pStatus) {
		pStatus->ReturnValue = r;
		pStatus->Done = 1;
	}
	WM_DeleteWindow(hDialog);
}
int     GUI_ExecCreatedDialog(WM_HWIN hDialog) {
	WM_DIALOG_STATUS DialogStatus = { 0 };
	/* Let window know how to send feedback (close info & return value) */
	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		GUI_Exec();
	}
	return DialogStatus.ReturnValue;
}
/*********************************************************************
*
*       GUI_ExecDialogbox
*
* Purpose:
*  Create and execute a dialog
*  The name is somewhat confusing, it should really be something like
*  GUI_CreateExecDialog. However, we keep it like that for compatibility
*
*/
int GUI_ExecDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget,
					  int NumWidgets, WM_CALLBACK *cb, WM_HWIN hParent, int x0, int y0) {
	WM_HWIN hDialog;
	hDialog = GUI_CreateDialogBox(paWidget, NumWidgets, cb, hParent, x0, y0);
	return GUI_ExecCreatedDialog(hDialog);
}
