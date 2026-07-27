#include <stdio.h>

#include "GUI.h"
#include "DIALOG.h"

import TUX.Widget;
import TUX.Widget.CheckBox;
import TUX.Widget.Menu;
import TUX.Widget.ListView;
import TUX.Widget.Frame;

static bool _MultiSel = false, _OwnerDrawn = true;
const RGBC ColorsSmilie0[]{ RGB_WHITE, RGB_BLACK, RGB_RED };
const GUI_LOGPALETTE PalSmilie0{ 3, 1, &ColorsSmilie0[0] };
const uint8_t acSmilie0[]{
________,XXXXXXXX,XX______,________,
______XX,oooooooo,ooXX____,________,
____XXoo,oooooooo,ooooXX__,________,
__XXooXX,XXoooooo,XXXXooXX,________,
XXooooXX,XXoooooo,XXXXoooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,oooooooo,oooooooo,XX______,
__XXoooo,ooXXXXXX,ooooooXX,________,
____XXoo,XXoooooo,XXooXX__,________,
______XX,oooooooo,ooXX____,________,
________,XXXXXXXX,XX______,________ };
CBITMAP bmSmilie0{
	/* Size */ 13, 13,
	/* BytesPerLine */ 4,
	/* BitsPerPixel */ 2,
	acSmilie0, &PalSmilie0
};

const RGBC ColorsSmilie1[]{ RGB_WHITE, RGB_BLACK, RGB_YELLOW };
const GUI_LOGPALETTE PalSmilie1{ 3, 1, &ColorsSmilie1[0] };
const uint8_t acSmilie1[]{
________,XXXXXXXX,XX______,________,
______XX,oooooooo,ooXX____,________,
____XXoo,oooooooo,ooooXX__,________,
__XXooXX,XXoooooo,XXXXooXX,________,
XXooooXX,XXoooooo,XXXXoooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,ooooXXoo,oooooooo,XX______,
XXoooooo,oooooooo,oooooooo,XX______,
__XXoooo,XXoooooo,XXooooXX,________,
____XXoo,ooXXXXXX,ooooXX__,________,
______XX,oooooooo,ooXX____,________,
________,XXXXXXXX,XX______,________ };
CBITMAP bmSmilie1{
	/* Size */ 13, 13,
	/* BytesPerLine */ 4,
	/* BitsPerPixel */ 2,
	acSmilie1, &PalSmilie1
};

static const GUI_ConstString _ListBox[]{
  "English", "Deutsch", nullptr
};
#define GUI_ID_MULTIEDIT0  GUI_ID_USER + 0x00
#define GUI_ID_CHECK0      GUI_ID_USER + 0x01
#define GUI_ID_CHECK1      GUI_ID_USER + 0x02
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[]{
	{ FRAMEWIN_CreateIndirect  , "Owner drawn list box" , 0                 , 50  , 50  , 220  , 175  , FRAMEWIN_CF_MOVEABLE       },
	{ LISTBOX_CreateIndirect   , ""                     , GUI_ID_MULTIEDIT0 , 10  , 10  , 100  , 100  , 0                    , 100 },
	{ CHECKBOX_CreateIndirect  , ""                     , GUI_ID_CHECK0     , 120 , 10  , 0    , 0                                 },
	{ TEXT_CreateIndirect      , "Multi select"         , 0                 , 140 , 10  , 80   , 15   , TEXT_CF_LEFT               },
	{ CHECKBOX_CreateIndirect  , ""                     , GUI_ID_CHECK1     , 120 , 35  , 0    , 0                                 },
	{ TEXT_CreateIndirect      , "Owner drawn"          , 0                 , 140 , 35  , 80   , 15   , TEXT_CF_LEFT               },
	{ BUTTON_CreateIndirect    , "OK"                   , GUI_ID_OK         , 120 , 65  , 80   , 20                                },
	{ BUTTON_CreateIndirect    , "Cancel"               , GUI_ID_CANCEL     , 120 , 90  , 80   , 20                                }
};

static int _GetItemSizeY(LISTBOX_Obj *pObj, int ItemIndex) {
	int DistY = GUI_GetFontDistY() + 1;
	if (pObj->GetMulti()) {
		if (pObj->GetItemSel(ItemIndex))
			DistY += 8;
	}
	else if (pObj->GetSel() == ItemIndex)
		DistY += 8;
	return DistY;
}

static int _OwnerDraw(const WIDGET_ITEM_DRAW_INFO *pDrawItemInfo) {
	auto pObj = (LISTBOX_Obj *)pDrawItemInfo->hWin;
	int Index = pDrawItemInfo->ItemIndex;
	switch (pDrawItemInfo->Cmd) {
		case WIDGET_ITEM_GET_XSIZE: {
			char acBuffer[100];
			pObj->GetItemText(Index, acBuffer, sizeof(acBuffer));
			return GUI_GetStringDistX(acBuffer) + bmSmilie0.XSize + 16;
		}
		case WIDGET_ITEM_GET_YSIZE:
			return _GetItemSizeY(pObj, Index);
		case WIDGET_ITEM_DRAW:
		{
			int ColorIndex = 0;
			char acBuffer[100];
			RGBC aColor[4] = { RGB_BLACK, RGB_WHITE, RGB_WHITE, RGB_GRAY };
			RGBC aBkColor[4] = { RGB_WHITE, RGB_GRAY, RGB_DARKBLUE, RGB_GRAYL(0xC0) };
			bool IsDisabled = pObj->GetItemDisabled(pDrawItemInfo->ItemIndex);
			bool IsSelected = pObj->GetItemSel(Index);
			int MultiSel = pObj->GetMulti();
			int Sel = pObj->GetSel();
			int YSize = _GetItemSizeY(pObj, Index);
			/* Calculate color index */
			if (MultiSel)
				if (IsDisabled)
					ColorIndex = 3;
				else
					ColorIndex = IsSelected ? 2 : 0;
			else if (IsDisabled)
				ColorIndex = 3;
			else if (pDrawItemInfo->ItemIndex == Sel)
				ColorIndex = WM_HasFocus(pObj) ? 2 : 1;
			else
				ColorIndex = 0;
			/* Draw item */
			GUI_SetBkColor(aBkColor[ColorIndex]);
			GUI_SetColor(aColor[ColorIndex]);
			pObj->GetItemText(pDrawItemInfo->ItemIndex, acBuffer, sizeof(acBuffer));
			GUI_Clear();
			auto FontDistY = GUI_GetFontDistY();
			GUI_DispStringAt(acBuffer, pDrawItemInfo->x0 + bmSmilie0.XSize + 16, pDrawItemInfo->y0 + (YSize - FontDistY) / 2);
			/* Draw bitmap */
			auto pBm = MultiSel ? IsSelected ? &bmSmilie1 : &bmSmilie0 : (pDrawItemInfo->ItemIndex == Sel) ? &bmSmilie1 : &bmSmilie0;
			GUI_DrawBitmap(pBm, pDrawItemInfo->x0 + 7, pDrawItemInfo->y0 + (YSize - pBm->YSize) / 2);
			/* Draw focus rectangle */
			if (MultiSel && (pDrawItemInfo->ItemIndex == Sel)) {
				GUI_RECT rFocus;
				GUI_RECT rInside = WM_GetInsideRect(pObj);
				rFocus.x0 = pDrawItemInfo->x0;
				rFocus.y0 = pDrawItemInfo->y0;
				rFocus.x1 = rInside.x1;
				rFocus.y1 = pDrawItemInfo->y0 + YSize - 1;
				GUI_SetColor(RGB_WHITE - aBkColor[ColorIndex]);
				GUI_DrawFocusRect(rFocus, 0);
			}
			break;
		}
		default:
			return LISTBOX_Obj::OwnerDraw(pDrawItemInfo);
	}
	return 0;
}


static WM_Obj *_hMemDevFrame;
static WM_Obj *_hMemDevPane;
static WM_Obj *_hNoMemDevFrame;
static WM_Obj *_hNoMemDevPane;
static int _MemDevPhase;

static WM_PARAM _cbMemDevPane(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_PAINT:
		{
			int xSize = WM_GetWindowSizeX(hWin);
			int ySize = WM_GetWindowSizeY(hWin);
			int BarWidth = 36;
			int Span = xSize - BarWidth - 20;
			int XPos = 0;
			int MemDevOn = (hWin == _hMemDevPane);
			if (Span > 0) {
				int Phase = _MemDevPhase % (Span * 2);
				XPos = Phase > Span ? Span * 2 - Phase : Phase;
			}
			GUI_SetBkColor(RGB_WHITE);
			GUI_Clear();
			GUI_SetColor(RGB_DARKGRAY);
			GUI_DrawRect({ 0, 0, xSize - 1, ySize - 1 });
			GUI_SetColor(RGB_BLACK);
			GUI_DispStringAt(MemDevOn ? "MemDev ON" : "MemDev OFF", 8, 8);
			GUI_DispStringAt(MemDevOn ? "WM_CF_MEMDEV enabled" : "WM_CF_MEMDEV disabled", 8, 24);
			GUI_SetColor(RGB_GRAY);
			GUI_DrawRect({ 10, 48, xSize - 11, 72 });
			GUI_SetColor(MemDevOn ? RGB_GREEN : RGB_RED);
			GUI_FillRect({ 10 + XPos, 49, 10 + XPos + BarWidth, 71 });
			GUI_SetColor(RGB_BLUE);
			GUI_FillRect({ 10, ySize - 40, xSize - 11, ySize - 25 });
			GUI_SetColor(RGB_YELLOW);
			GUI_FillRect({ 10 + XPos / 2, ySize - 39, 35 + XPos / 2, ySize - 26 });
			GUI_SetColor(RGB_BLACK);
			GUI_DispStringAt("Animated redraw area", 8, ySize - 18);
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

static WM_Obj *_CreateMemDevFrame(int x0, int y0, const char *pTitle, int UseMemDev, WM_Obj **phPane) {
	WM_Obj *hFrame;
	WM_Obj *hClient;
	int xSize;
	int ySize;
	int Flags;
	Flags = WM_CF_SHOW;
	if (UseMemDev) {
		Flags |= WM_CF_MEMDEV;
	}
	hFrame = FRAMEWIN_CreateEx(x0, y0, 190, 180, 0, WM_CF_SHOW, FRAMEWIN_CF_MOVEABLE, 0, pTitle, 0);
	hClient = WM_GetClientWindow(hFrame);
	xSize = WM_GetWindowSizeX(hClient);
	ySize = WM_GetWindowSizeY(hClient);
	*phPane = WM_CreateWindowAsChild(0, 0, xSize, ySize, hClient, Flags, _cbMemDevPane, 0);
	return hFrame;
}

static WM_PARAM _cbCallback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	CHECKBOX_Obj *pItem;
	auto pListBox = (LISTBOX_Obj *)WM_GetDialogItem(hWin, GUI_ID_MULTIEDIT0);
	switch (MsgId) {
		case WM_INIT_DIALOG:
			pListBox->SetText(_ListBox);
			pListBox->AddString("Francis");
			pListBox->AddString("Japanese");
			pListBox->AddString("Italiano");
			pListBox->AddString("Espanol");
			pListBox->AddString("Greek");
			pListBox->AddString("Hebrew");
			pListBox->AddString("Dutch");
			pListBox->AddString("Other language ...");
			pListBox->SetScrollStepH(6);
			pListBox->SetAutoScrollH(1);
			pListBox->SetAutoScrollV(1);
			pListBox->SetOwnerDraw(_OwnerDraw);
			pItem = (CHECKBOX_Obj *)WM_GetDialogItem(hWin, GUI_ID_CHECK1);
			pItem->SetState(1);
			return 0;
		case WM_KEY: {
			const WM_KEY_INFO *pInfo = (const WM_KEY_INFO *)Data;
			switch (pInfo->Key) {
				case GUI_KEY_ESCAPE:
					GUI_EndDialog(hWin, 1);
					break;
				case GUI_KEY_ENTER:
					GUI_EndDialog(hWin, 0);
					break;
			}
			return 0;
		}
		case WM_TOUCH_CHILD:
			//WM_SetFocus(hListBox);
			return 0;
		case WM_NOTIFY_PARENT: {
			const WM_NOTIFY_INFO *pInfo = (const WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->pWinSrc); /* Id of widget */
			pItem = (CHECKBOX_Obj *)WM_GetDialogItem(hWin, Id);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_SEL_CHANGED:
					pListBox->InvalidateItem(LISTBOX_ALL_ITEMS);
					break;
				case WM_NOTIFICATION_RELEASED: /* React only if released */
					switch (Id) {
						case GUI_ID_OK:
							GUI_EndDialog(hWin, 0);
							break;
						case GUI_ID_CANCEL:
							GUI_EndDialog(hWin, 1);
							break;
						case GUI_ID_CHECK0:
							_MultiSel = !_MultiSel;
							pListBox->SetMulti(_MultiSel);
							WM_SetFocus(pListBox);
							pListBox->InvalidateItem(LISTBOX_ALL_ITEMS);
							break;
						case GUI_ID_CHECK1:
							_OwnerDrawn = !_OwnerDrawn;
							if (_OwnerDrawn)
								pListBox->SetOwnerDraw(_OwnerDraw);
							else
								pListBox->SetOwnerDraw(nullptr);
							pListBox->InvalidateItem(LISTBOX_ALL_ITEMS);
							break;
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

#define ID_MENU             (GUI_ID_USER +  0)
#define ID_MENU_FILE_NEW    (GUI_ID_USER +  1)
#define ID_MENU_FILE_OPEN   (GUI_ID_USER +  2)
#define ID_MENU_FILE_CLOSE  (GUI_ID_USER +  3)
#define ID_MENU_FILE_EXIT   (GUI_ID_USER +  4)
#define ID_MENU_FILE_RECENT (GUI_ID_USER +  5)
#define ID_MENU_RECENT_0    (GUI_ID_USER +  6)
#define ID_MENU_RECENT_1    (GUI_ID_USER +  7)
#define ID_MENU_RECENT_2    (GUI_ID_USER +  8)
#define ID_MENU_RECENT_3    (GUI_ID_USER +  9)
#define ID_MENU_EDIT_UNDO   (GUI_ID_USER + 10)
#define ID_MENU_EDIT_REDO   (GUI_ID_USER + 11)
#define ID_MENU_EDIT_COPY   (GUI_ID_USER + 12)
#define ID_MENU_EDIT_PASTE  (GUI_ID_USER + 13)
#define ID_MENU_EDIT_DELETE (GUI_ID_USER + 14)
#define ID_MENU_HELP_ABOUT  (GUI_ID_USER + 15)

static void _AddMenuItem(MENU_Obj *pMenu, MENU_Obj *pSubmenu, const char *pText, uint16_t Id, uint16_t Flags) {
	MENU_ITEM_DATA Item;
	Item.pText = pText;
	Item.pSubmenu = pSubmenu;
	Item.Flags = Flags;
	Item.Id = Id;
	pMenu->AddItem(&Item);
}
/*********************************************************************
*
*       MainTask
*
*       Demonstrates a owner drawn list box
*
**********************************************************************
*/
static void _CreateMenu(FRAMEWIN_Obj *pParent) {
	//
	// Create main menu
	//
	auto hMenu = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_HORIZONTAL, ID_MENU);
	//
	// Create sub menus
	//
	auto hMenuFile = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	auto hMenuEdit = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	auto hMenuHelp = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	auto hMenuRecent = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	//
	// Add menu items to menu &#39;Recent&#39;
	//
	_AddMenuItem(hMenuRecent, 0, "File 1", ID_MENU_RECENT_0, 0);
	_AddMenuItem(hMenuRecent, 0, "File 2", ID_MENU_RECENT_1, 0);
	_AddMenuItem(hMenuRecent, 0, "File 3", ID_MENU_RECENT_2, 0);
	_AddMenuItem(hMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	_AddMenuItem(hMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	_AddMenuItem(hMenuRecent, 0, "File 4", ID_MENU_RECENT_3, 0);
	//
	// Add menu items to menu &#39;File&#39;
	//
	_AddMenuItem(hMenuFile, 0, "New", ID_MENU_FILE_NEW, 0);
	_AddMenuItem(hMenuFile, 0, "Open", ID_MENU_FILE_OPEN, 0);
	_AddMenuItem(hMenuFile, 0, "Close", ID_MENU_FILE_CLOSE, MENU_IF_DISABLED);
	_AddMenuItem(hMenuFile, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(hMenuFile, hMenuRecent, "Files...", ID_MENU_FILE_RECENT, 0);
	_AddMenuItem(hMenuFile, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(hMenuFile, 0, "Exit", ID_MENU_FILE_EXIT, 0);
	//
	// Add menu items to menu &#39;Edit&#39;
	//
	_AddMenuItem(hMenuEdit, 0, "Undo", ID_MENU_EDIT_UNDO, 0);
	_AddMenuItem(hMenuEdit, 0, "Redo", ID_MENU_EDIT_REDO, 0);
	_AddMenuItem(hMenuEdit, 0, 0, 0, MENU_IF_SEPARATOR);
	_AddMenuItem(hMenuEdit, 0, "Copy", ID_MENU_EDIT_COPY, 0);
	_AddMenuItem(hMenuEdit, 0, "Paste", ID_MENU_EDIT_PASTE, 0);
	_AddMenuItem(hMenuEdit, 0, "Delete", ID_MENU_EDIT_DELETE, 0);
	//
	// Add menu items to menu &#39;Help&#39;
	//
	_AddMenuItem(hMenuHelp, 0, "About", ID_MENU_HELP_ABOUT, 0);
	//
	// Add menu items to main menu
	//
	_AddMenuItem(hMenu, hMenuFile, "File", 0, 0);
	_AddMenuItem(hMenu, hMenuEdit, "Edit", 0, 0);
	_AddMenuItem(hMenu, hMenuHelp, "Help", 0, 0);
	//
	// Attach menu to parent window
	//
	pParent->AddMenu(hMenu);
}

void _Create() {
}

/*********************************************************************
*
*       _TestListView
*
*       Demonstrates ListView widget
*
**********************************************************************
*/

#define ID_LISTVIEW_TEST    (GUI_ID_USER + 100)

static const GUI_WIDGET_CREATE_INFO _aListViewDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect  , "ListView Test"      , 0                 , 50  , 50  , 320 , 240 , FRAMEWIN_CF_MOVEABLE       },
	{ LISTVIEW_CreateIndirect  , ""                   , ID_LISTVIEW_TEST  , 10  , 10  , 290 , 160 , 0                          },
	{ BUTTON_CreateIndirect    , "Add Row"            , GUI_ID_USER + 101 , 10  , 180 , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Delete Row"         , GUI_ID_USER + 102 , 100 , 180 , 80  , 25                             },
	{ BUTTON_CreateIndirect    , "Close"              , GUI_ID_CANCEL     , 190 , 180 , 80  , 25                             }
};

static WM_PARAM _cbListViewTest(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
	switch (MsgId) {
		case WM_INIT_DIALOG: {
			auto pListView = (LISTVIEW_Obj *)WM_GetDialogItem(hWin, ID_LISTVIEW_TEST);
			// Add columns
			pListView->AddColumn(80, "Name", TEXTALIGN_LEFT);
			pListView->AddColumn(60, "Age", TEXTALIGN_RIGHT);
			pListView->AddColumn(120, "City", TEXTALIGN_LEFT);
			// Add rows
			const char *row1[] = { "Alice", "25", "New York", nullptr };
			const char *row2[] = { "Bob", "30", "London", nullptr };
			const char *row3[] = { "Charlie", "35", "Tokyo", nullptr };
			pListView->AddRow(row1);
			pListView->AddRow(row2);
			pListView->AddRow(row3);
			// Set grid visible
			pListView->SetGridVis(1);
			break;
		}
		case WM_NOTIFY_PARENT:
		{
			auto pInfo = (WM_NOTIFY_INFO *)Data;
			int Id = WM_GetId(pInfo->pWinSrc);
			auto pListView = (LISTVIEW_Obj *)WM_GetDialogItem(hWin, ID_LISTVIEW_TEST);
			switch (pInfo->Notification) {
				case WM_NOTIFICATION_RELEASED:
					switch (Id) {
						case GUI_ID_USER + 101: // Add Row
						{
							static int rowCount = 1;
							char name[32], age[32];
							sprintf(name, "User %d", rowCount);
							sprintf(age, "%d", 20 + rowCount);
							const char *newRow[] = { name, age, "Paris", nullptr };
							pListView->AddRow(newRow);
							rowCount++;
							break;
						}
						case GUI_ID_USER + 102: // Delete Row
						{
							int sel = pListView->GetSel();
							if (sel >= 0) {
								pListView->DeleteRow(sel);
							}
							break;
						}
						case GUI_ID_CANCEL:
							GUI_EndDialog(hWin, 0);
							break;
					}
					break;
				case WM_NOTIFICATION_SEL_CHANGED:
					if (Id == ID_LISTVIEW_TEST) {
						int sel = pListView->GetSel();
						// Can add code to respond to selection change
					}
					break;
			}
			return 0;
		}
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}

void _TestListView() {
	WM_Obj *hDialog = GUI_CreateDialogBox(_aListViewDialogCreate, GUI_COUNTOF(_aListViewDialogCreate), &_cbListViewTest, 0, 0, 0);
	WM_DIALOG_STATUS DialogStatus = { 0 };
	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		GUI_Exec();
	}
}

int main(void) {
	GUI_Init();
	GUI_CURSOR_Show();
	WM_SetDesktopColor(RGB_GRAY);

	// Test ListView widget
	_TestListView();

	for (;;) {
		auto hDialog = (FRAMEWIN_Obj *)GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
		hDialog->AddMinButton();
		hDialog->AddMaxButton();
		_CreateMenu(hDialog);
		_MemDevPhase = 0;
		_hMemDevFrame = _CreateMemDevFrame(280, 50, "MemDev ON", 1, &_hMemDevPane);
		_hNoMemDevFrame = _CreateMemDevFrame(480, 50, "MemDev OFF", 0, &_hNoMemDevPane);

		WM_DIALOG_STATUS DialogStatus = { 0 };
		/* Let window know how to send feedback (close info & return value) */
		GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
		while (!DialogStatus.Done) {
			_MemDevPhase = GUI_GetTime() / 20;
			if (_hMemDevPane)
				WM_Invalidate(_hMemDevPane);
			if (_hNoMemDevPane)
				WM_Invalidate(_hNoMemDevPane);
			GUI_Exec();
		}
		if (_hMemDevFrame)
			WM_DeleteWindow(_hMemDevFrame);
		if (_hNoMemDevFrame)
			WM_DeleteWindow(_hNoMemDevFrame);
		_hMemDevFrame = 0;
		_hMemDevPane = 0;
		_hNoMemDevFrame = 0;
		_hNoMemDevPane = 0;
	}
	return 0;
}
