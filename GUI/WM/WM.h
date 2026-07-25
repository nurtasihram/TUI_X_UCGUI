#pragma once

#include "GUI_ConfDefaults.h"
#include "GUIType.h"      /* Needed because of typedefs only */

/* Support for transparency. Switching it off makes Wm smaller and faster */
#ifndef WM_SUPPORT_TRANSPARENCY
#define WM_SUPPORT_TRANSPARENCY 1
#endif

/* Make sure we actually have configured windows. If we have not,
there is no point for a windows manager and it will therefor not
generate any code !
*/

#ifndef WM_SUPPORT_OBSTRUCT
#define WM_SUPPORT_OBSTRUCT 1
#endif

/* Send a message if visibility of a window has changed */
#ifndef WM_SUPPORT_NOTIFY_VIS_CHANGED
#define WM_SUPPORT_NOTIFY_VIS_CHANGED 0
#endif

/*********************************************************************
*
*               Messages Ids
* The following is the list of windows messages.
*/
enum WM_MSGID : int {
	 /* The first message received, right after client has actually been created */
	 WM_CREATE                  = 1 ,
	 /* window has been moved */
	 WM_MOVE                    = 3 ,
	 /* Is sent to a window after its size has changed */
	 WM_SIZE                    = 5 ,
	 /* Delete (Destroy) command: This tells the client to free its data strutures since the window it is associates with no longer exists.*/
	 WM_DELETE                  = 11,
	 /* Touch screen message */
	 WM_TOUCH                   = 12,
	 /* Touch screen message to ancestors */
	 WM_TOUCH_CHILD             = 13,
	 /* Key has been pressed */
	 WM_KEY                     = 14,
	 /* Repaint window (because content is (partially) invalid */
	 WM_PAINT                   = 15,
	 /* Mouse has moved, no key pressed */
	 WM_MOUSEOVER               = 16,
	 /* Pointer input device state has changed */
	 WM_PID_STATE_CHANGED       = 17,
	 /* get inside rectangle: client rectangle minus pixels lost to effect */
	 WM_GET_INSIDE_RECT         = 20,
	 /* Get id of widget */
	 WM_GET_ID                  = 21,
	 /* Set id of widget */
	 WM_SET_ID                  = 22,
	 /* Get window handle of client window. Default is the same as window */
	 WM_GET_CLIENT_WINDOW       = 23,
	 /* Let window know that mouse capture is over */
	 WM_CAPTURE_RELEASED        = 24,
	 /* Inform dialog that it is ready for init */
	 WM_INIT_DIALOG             = 30,
	 /* Inform window that it has gotten or lost the focus */
	 WM_SET_FOCUS               = 31,
	 /* Find out if window can accept the focus */
	 WM_GET_ACCEPT_FOCUS        = 32,
	 
	 WM_NOTIFY_CHILD_HAS_FOCUS  = 33,
	 /* Return back ground color (only frame window and similar) */
	 WM_GET_BKCOLOR             = 34,
	 /* Query state of scroll bar */
	 WM_GET_SCROLL_STATE        = 35,
	 /* Set scroll info ... only effective for scrollbars */
	 WM_SET_SCROLL_STATE        = 36,
	 /* Client area may have changed */
	 WM_NOTIFY_CLIENTCHANGE     = 37,
	 /* Notify parent. Information is detailed as notification code */
	 WM_NOTIFY_PARENT           = 38,
	 /* Enable or disable widget */
	 WM_NOTIFY_ENABLE           = 40,
	 /* Visibility of a window has or may have changed */
	 WM_NOTIFY_VIS_CHANGED      = 41,
	 /* Set or get dialog status */
	 WM_HANDLE_DIALOG_STATUS    = 42,
	 /* Send to all siblings and children of a radio control when selection changed */
	 WM_GET_RADIOGROUP          = 43,
	 /* Send to owner window of menu widget */	
	WM_MENU                    = 44,
};

#define WM_TIMER                    0x0113  /* Timer has expired */
#define WM_WIDGET                   0x0300  /* 256 messages reserved for Widget messages */
#define WM_USER                     0x0400  /* Reserved for user messages ...  */

/*********************************************************************
*
*               Notification codes
*
* The following is the list of notification codes send
* with the WM_NOTIFY_PARENT message
*/
enum WM_NOTIFICATION : int { 
	 WM_NOTIFICATION_CLICKED           =  1,
	 WM_NOTIFICATION_RELEASED          =  2,
	 WM_NOTIFICATION_MOVED_OUT         =  3,
	 WM_NOTIFICATION_SEL_CHANGED       =  4,
	 WM_NOTIFICATION_VALUE_CHANGED     =  5,
	 WM_NOTIFICATION_SCROLLBAR_ADDED   =  6,
	 WM_NOTIFICATION_CHILD_DELETED     =  7,
	 WM_NOTIFICATION_GOT_FOCUS         =  8,
	 WM_NOTIFICATION_LOST_FOCUS        =  9,
	 WM_NOTIFICATION_SCROLL_CHANGED    =  10
};

#define WM_NOTIFICATION_WIDGET             11      /* Space for widget defined notifications */
#define WM_NOTIFICATION_USER               16      /* Space for  application (user) defined notifications */

/*********************************************************************
*
*           Window create flags.
* These flags can be passed to the create window
* function as flag-parameter. The flags are combinable using the
* binary or operator.
*/
enum WM_CF : uint16_t {
	 /* Has transparency. Needs to be defined for windows which do not fill the entire section of their (client) rectangle. */
	 WM_CF_HASTRANS         = 1 << 0, 
	 /* Hide window after creation (default !) */
	 WM_CF_HIDE             = 0 << 1, 
	 /* Show window after creation */
	 WM_CF_SHOW             = 1 << 1, 
	 /* Use memory device for redraws */
	 WM_CF_MEMDEV           = 1 << 2, 
	 /* Stay on top */
	 WM_CF_STAYONTOP        = 1 << 3, 
	 /* Disabled: Does not receive PID (mouse & touch) input */
	 WM_CF_DISABLED         = 1 << 4, 

	 /* Create only flags ... Not available as status flags */
	
	 /* If automatic activation upon creation of window is desired */
	 WM_CF_ACTIVATE         = 1 << 5, 
	 /* Put window in foreground after creation (default !) */
	 WM_CF_FGND             = 0 << 6, 
	 /* Put window in background after creation */
	 WM_CF_BGND             = 1 << 6, 

	 /* Anchor flags */
	
	 /* Right anchor ... If parent is resized, distance to right will remain const (left is default) */
	 WM_CF_ANCHOR_RIGHT     = 1 << 7, 
	 /* Bottom anchor ... If parent is resized, distance to bottom will remain const (top is default) */
	 WM_CF_ANCHOR_BOTTOM    = 1 << 8, 
	 /* Left anchor ... If parent is resized, distance to left will remain const (left is default) */
	 WM_CF_ANCHOR_LEFT      = 1 << 9, 
	 /* Top anchor ... If parent is resized, distance to top will remain const (top is default) */
	 WM_CF_ANCHOR_TOP       = 1 << 10,

	 /* Constant outline. This is relevant for transparent windows only. If a window is transparent and does not have a constant outline, its background is invalided instead of the window itself. This causes add. computation time when redrawing. */
	 WM_CF_CONST_OUTLINE    = 1 << 11,
	 WM_CF_LATE_CLIP        = 1 << 12,
	 WM_CF_MEMDEV_ON_REDRAW = 1 << 13,
	 WM_CF_RESERVED3        = 1 << 14,
	 WM_CF_RESERVED4        = 1 << 15
};

using WM_HMEM = GUI_HMEM;

typedef uintptr_t WM_PARAM;

struct WM_KEY_INFO {
	int16_t Key, PressedCnt;
};

struct WM_SCROLL_STATE {
	int16_t NumItems = 0, v = 0, PageSize = 0;
	inline bool operator!=(const WM_SCROLL_STATE &other) const {
		return NumItems != other.NumItems || v != other.v || PageSize != other.PageSize;
	}
	inline bool operator==(const WM_SCROLL_STATE &other) const {
		return !(*this != other);
	}
};

struct WM_DIALOG_STATUS {
	int16_t Done;
	int16_t ReturnValue;
};

struct WM_PID_STATE_CHANGED_INFO {
	int16_t x, y;
	uint8_t State, StatePrev;
};

#define WM_UNATTACHED  ((WM_Obj *)-1)                        /* Do not attach to a window */
typedef WM_PARAM WM_CALLBACK(struct WM_Obj *pWin, int MsgId, WM_PARAM Data);
struct WM_Obj {
	GUI_RECT Rect;        /* outer dimensions of window */
	GUI_RECT InvalidRect; /* invalid rectangle */
	WM_CALLBACK *cb;      /* ptr to notification callback */
	WM_Obj *pNextLin;     /* Next window in linear list */
	WM_Obj *pParent;
	WM_Obj *pFirstChild;
	WM_Obj *pNext;
	uint16_t Status; /* Some status flags */
};

struct WM_NOTIFY_INFO {
	int Notification;
	WM_Obj *pWinSrc;
};

typedef void WM_tfForEach(WM_Obj *pWin, void *pData);

void WM_Activate(void);
void WM_Deactivate(void);

void WM_Init(void);
int  WM_Exec(void);  /* Execute all jobs ... Return 0 if nothing was done. */
int  WM_Exec1(void); /* Execute one job  ... Return 0 if nothing was done. */

void WM_SetCapture(WM_Obj *pObj, int AutoRelease);
void WM_SetCaptureMove(WM_Obj *pWin, const GUI_PID_STATE *pState, int MinVisibility); /* Not yet documented */
void WM_ReleaseCapture(void);

uint16_t WM_SetCreateFlags(uint16_t Flags);

void    WM_AttachWindow(WM_Obj *pWin, WM_Obj *pParent);
void    WM_AttachWindowAt(WM_Obj *pWin, WM_Obj *pParent, int x, int y);
int     WM_CheckScrollPos(WM_SCROLL_STATE *pScrollState, int Pos, int LowerDist, int UpperDist); /* not to be documented (may change in future version) */
void    WM_ClrHasTrans(WM_Obj *pWin);
WM_Obj *WM_CreateWindow(int x0, int y0, int xSize, int ySize, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes);
WM_Obj *WM_CreateWindowAsChild(int x0, int y0, int xSize, int ySize, WM_Obj *pWinParent, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes);
void    WM_DeleteWindow(WM_Obj *pWin);
void    WM_DetachWindow(WM_Obj *pWin);
int     WM_GetHasTrans(WM_Obj *pWin);
WM_Obj *WM_GetFocussedWindow(void);
void    WM_HideWindow(WM_Obj *pWin);
void    WM_InvalidateArea(const GUI_RECT *pRect);
void    WM_InvalidateRect(WM_Obj *pWin, const GUI_RECT *pRect);
void    WM_Invalidate(WM_Obj *pWin);
void    WM_InvalidateDescs(WM_Obj *pWin);    /* not to be documented (may change in future version) */
bool    WM_IsEnabled(WM_Obj *pObj);
bool    WM_IsFocussable(WM_Obj *pWin);
bool    WM_IsVisible(WM_Obj *pWin);
bool    WM_IsWindow(WM_Obj *pWin);    /* Check validity */
void    WM_SetHasTrans(WM_Obj *pWin);
void    WM_SetId(WM_Obj *pObj, int Id);
void    WM_SetTransState(WM_Obj *pWin, unsigned State);
void    WM_ShowWindow(WM_Obj *pWin);
void    WM_ValidateRect(WM_Obj *pWin, const GUI_RECT *pRect);
void    WM_ValidateWindow(WM_Obj *pWin);
int     WM_GetInvalidRect(WM_Obj *pWin, GUI_RECT *pRect);
void    WM_SetStayOnTop(WM_Obj *pWin, int OnOff);
int     WM_GetStayOnTop(WM_Obj *pWin);
void    WM_SetAnchor(WM_Obj *pWin, uint16_t AnchorFlags);

/* Move/resize windows */
void WM_MoveWindow(WM_Obj *pWin, int dx, int dy);
void WM_ResizeWindow(WM_Obj *pWin, int dx, int dy);
void WM_MoveTo(WM_Obj *pWin, int x, int y);
void WM_MoveChildTo(WM_Obj *pWin, int x, int y);
void WM_SetSize(WM_Obj *pWin, int XSize, int YSize);
int  WM_SetXSize(WM_Obj *pWin, int xSize);
int  WM_SetYSize(WM_Obj *pWin, int ySize);
int  WM_CreateTimer(WM_Obj *pWin, int UserID, int Period, int Mode); /* not to be documented (may change in future version) */
void WM_DeleteTimer(WM_Obj *pWin, int UserId); /* not to be documented (may change in future version) */
int  WM_SetScrollbarH(WM_Obj *pWin, int OnOff); /* not to be documented (may change in future version) */
int  WM_SetScrollbarV(WM_Obj *pWin, int OnOff); /* not to be documented (may change in future version) */

/* Diagnostics */
int WM_GetNumWindows(void);
int WM_GetNumInvalidWindows(void);

/* Scroll state related functions */
int  WM_SetScrollValue(WM_SCROLL_STATE *pScrollState, int v); /* not to be documented (may change in future version) */
void WM_CheckScrollBounds(WM_SCROLL_STATE *pScrollState); /* not to be documented (may change in future version) */

/* Set (new) callback function */
WM_CALLBACK *WM_SetCallback(WM_Obj *Win, WM_CALLBACK *cb);

/* Get size/origin of a window */
GUI_RECT WM_GetClientRect();
GUI_RECT WM_GetClientRect(WM_Obj *pWin);
GUI_RECT WM_GetInsideRect();
GUI_RECT WM_GetInsideRect(WM_Obj *pWin);
GUI_RECT WM_GetWindowRect();
GUI_RECT WM_GetWindowRect(WM_Obj *pWin);

void WM_GetInsideRectExScrollbar(WM_Obj *pWin, GUI_RECT *pRect); /* not to be documented (may change in future version) */
int  WM_GetOrgX(void);
int  WM_GetOrgY(void);
int  WM_GetWindowOrgX(WM_Obj *pWin);
int  WM_GetWindowOrgY(WM_Obj *pWin);
int  WM_GetWindowSizeX(WM_Obj *pWin);
int  WM_GetWindowSizeY(WM_Obj *pWin);
WM_Obj *WM_GetFirstChild(WM_Obj *pWin);
WM_Obj *WM_GetNextSibling(WM_Obj *pWin);
WM_Obj *WM_GetParent(WM_Obj *pWin);
WM_Obj *WM_GetPrevSibling(WM_Obj *pWin);
int     WM_GetId(WM_Obj *pWin);
WM_Obj *WM_GetScrollbarV(WM_Obj *pWin);
WM_Obj *WM_GetScrollbarH(WM_Obj *pWin);
WM_Obj *WM_GetScrollPartner(WM_Obj *pWin);
WM_Obj *WM_GetClientWindow(WM_Obj *pObj);
RGBC WM_GetBkColor(WM_Obj *pObj);

/* Change Z-Order of windows */
void WM_BringToBottom(WM_Obj *pWin);
void WM_BringToTop(WM_Obj *pWin);

RGBC WM_SetDesktopColor(RGBC Color);

/* Select window used for drawing operations */
WM_Obj *WM_SelectWindow(WM_Obj *pWin);
WM_Obj *WM_GetActiveWindow(void);
void    WM_Paint(WM_Obj *pObj);

/* Get foreground/background windows */
WM_Obj *WM_GetDesktopWindow(void);

/* Reduce clipping area of a window */
const GUI_RECT *WM_SetUserClipRect(const GUI_RECT *pRect);
void            WM_SetDefault(void);

/* Use of memory devices */
void WM_EnableMemdev(WM_Obj *pWin);
void WM_DisableMemdev(WM_Obj *pWin);

int WM_OnKey(int Key, int Pressed);

/******************************************************************
*
*           Message related funcions
*
*******************************************************************
	Please note that some of these functions do not yet show up in the
	documentation, as they should not be required by application program.
*/

void      WM_NotifyParent(WM_Obj *pWin, int Notification);
WM_PARAM  WM_SendMessage(WM_Obj *pWin, int MsgId, WM_PARAM Data);
void      WM_SendMessageNoPara(WM_Obj *pWin, int MsgId); /* not to be documented (may change in future versionumented */

WM_PARAM  WM_DefaultProc(WM_Obj *pWin, int MsgId, WM_PARAM Data);

void      WM_SetScrollState(WM_Obj *pWin, const WM_SCROLL_STATE *pState);
void      WM_SetEnableState(WM_Obj *pItem, int State);
bool      WM_HasCaptured(WM_Obj *pWin);
bool      WM_HasFocus(WM_Obj *pWin);
int       WM_SetFocus(WM_Obj *pWin);
WM_Obj *WM_SetFocusOnNextChild(WM_Obj *pParent);     /* Set the focus to the next child */
WM_Obj *WM_SetFocusOnPrevChild(WM_Obj *pParent);     /* Set the focus to the previous child */
WM_Obj *WM_GetDialogItem(WM_Obj *pWin, int Id);
void      WM_EnableWindow(WM_Obj *pWin);
void      WM_DisableWindow(WM_Obj *pWin);
void      WM_GetScrollState(WM_Obj *pObj, WM_SCROLL_STATE *pScrollState);
int       WM_GetUserData(WM_Obj *pWin, void *pDest, int SizeOfBuffer);
int       WM_SetUserData(WM_Obj *pWin, const void *pSrc, int SizeOfBuffer);

int       WM_HandlePID(void);
WM_Obj *WM_Screen2hWin(int x, int y);
WM_Obj *WM_Screen2hWinEx(WM_Obj *pStop, int x, int y);
void      WM_ForEachDesc(WM_Obj *pWin, WM_tfForEach *pcb, void *pData);

#pragma region IVR
bool WM__InitIVRSearch(GUI_RECT rcMax);
bool WM__GetNextIVR(void);

#define WM_ITERATE_START(r)   \
	if (WM__InitIVRSearch(r)) \
		do {
#define WM_ITERATE_END()          \
	} while (WM__GetNextIVR());

#define WM_ADDORGX(x0)    x0 += GUI_Context.Off.x
#define WM_ADDORGY(y0)    y0 += GUI_Context.Off.y
#define WM_ADDORG(x0,y0) WM_ADDORGX(x0); WM_ADDORGY(y0)

#define WM_SUBORGX(x0)    x0 -= GUI_Context.Off.x
#define WM_SUBORGY(y0)    y0 -= GUI_Context.Off.y
#define WM_SUBORG(x0,y0) WM_SUBORGX(x0); WM_SUBORGY(y0)
#pragma endregion
