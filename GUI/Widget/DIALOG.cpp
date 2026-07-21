
#include "DIALOG.h"
#include "WIDGET.h"
#include "WM_Intern.h"

/* Define colors */
#define DIALOG_BKCOLOR0_DEFAULT RGB_GRAYL(0xc0)
static RGBC _BkColor = DIALOG_BKCOLOR0_DEFAULT;
RGBC DIALOG_GetBkColor(void) {
	return _BkColor;
}
RGBC DIALOG_SetBkColor(RGBC BkColor) {
	RGBC r;
	r = _BkColor;
	_BkColor = BkColor;
	return r;
}
void GUI_SetDialogStatusPtr(WM_HWIN hDialog, WM_DIALOG_STATUS *pDialogStatus) {
	WM_SendMessage(hDialog, WM_HANDLE_DIALOG_STATUS, (WM_PARAM)pDialogStatus);
}
WM_DIALOG_STATUS *GUI_GetDialogStatusPtr(WM_HWIN hDialog) {
	return (WM_DIALOG_STATUS *)WM_SendMessage(hDialog, WM_HANDLE_DIALOG_STATUS, 0);
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
	while (!DialogStatus.Done)
		GUI_Exec();
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
