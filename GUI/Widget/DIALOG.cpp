#include "DIALOG.h"
#include "WM_Intern.h"

import TUX.Widget;

/* Define colors */
void GUI_SetDialogStatusPtr(WM_Obj *pDialog, WM_DIALOG_STATUS *pDialogStatus) {
	pDialog->SendMessage(WM_HANDLE_DIALOG_STATUS, (WM_PARAM)pDialogStatus);
}
WM_DIALOG_STATUS *GUI_GetDialogStatusPtr(WM_Obj *pDialog) {
	return (WM_DIALOG_STATUS *)pDialog->SendMessage(WM_HANDLE_DIALOG_STATUS, 0);
}
WM_Obj * GUI_CreateDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WM_Obj * hParent,
							int x0, int y0) {
	auto pDialog = paWidget->pfCreateIndirect(paWidget, hParent, x0, y0, cb);     /* Create parent window */
	auto pDialogClient = WM_GetClientWindow(pDialog);
	((WIDGET *)pDialog)->AddStates(paWidget->Flags);
	pDialog->ShowWindow();
	pDialogClient->ShowWindow();
	while (--NumWidgets > 0) {
		paWidget++;
		auto pChild = paWidget->pfCreateIndirect(paWidget, pDialogClient, 0, 0, 0);     /* Create child window */
		pChild->ShowWindow();
	}
	WM_SetFocusOnNextChild(pDialog);     /* Set the focus to the first child */
	WM_SendMessageNoPara(pDialogClient, WM_INIT_DIALOG);
	return pDialog;
}
void GUI_EndDialog(WM_Obj *pDialog, int r) {
	WM_DIALOG_STATUS *pStatus;
	pStatus = GUI_GetDialogStatusPtr(pDialog);
	if (pStatus) {
		pStatus->ReturnValue = r;
		pStatus->Done = 1;
	}
	WM_DeleteWindow(pDialog);
}
int GUI_ExecCreatedDialog(WM_Obj *pDialog) {
	WM_DIALOG_STATUS DialogStatus = { 0 };
	/* Let window know how to send feedback (close info & return value) */
	GUI_SetDialogStatusPtr(pDialog, &DialogStatus);
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
					  int NumWidgets, WM_CALLBACK *cb, WM_Obj *pParent, int x0, int y0) {
	auto pDialog = GUI_CreateDialogBox(paWidget, NumWidgets, cb, pParent, x0, y0);
	return GUI_ExecCreatedDialog(pDialog);
}
