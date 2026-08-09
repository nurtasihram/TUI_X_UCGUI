#include "DIALOG.h"
#include "WM_Intern.h"

import TUX.Widget;

#pragma region Dialog
WObj * GUI_CreateDialogBox(const GUI_WIDGET_CREATE_INFO *paWidget, int NumWidgets, WM_CALLBACK *cb, WObj * hParent,
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
	pDialogClient->Require(WM_INIT_DIALOG);
	return pDialog;
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
					  int NumWidgets, WM_CALLBACK *cb, WObj *pParent, int x0, int y0) {
	auto pDialog = GUI_CreateDialogBox(paWidget, NumWidgets, cb, pParent, x0, y0);
	return pDialog->DialogExec();
}
#pragma endregion