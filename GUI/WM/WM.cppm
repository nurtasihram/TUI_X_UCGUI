module;

#include <stdint.h>

export module TUX.WindowTypes;

export {

#pragma region Messages & Notifications	
/*********************************************************************
*
*               Messages Ids
* The following is the list of windows messages.
*/
enum WM_MSGID : int {
	 WM_NONE                    = 0,
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
	 /* Set or get dialog status */
	 WM_HANDLE_DIALOG_STATUS    = 41,
	 /* Send to all siblings and children of a radio control when selection changed */
	 WM_GET_RADIOGROUP          = 42,
	 /* Send to owner window of menu widget */	
	 WM_MENU                    = 43,
};
template<auto code> constexpr int16_t WM_WIDGET = 0x0300 + code;
template<auto code> constexpr int16_t WM_USER   = 0x0400 + code;

/*********************************************************************
*
*               Notification codes
*
* The following is the list of notification codes send
* with the WM_NOTIFY_PARENT message
*/
enum WM_NOTIFICATION : int {
	WM_NOTIFICATION_START            = 0,
	WM_NOTIFICATION_CLICKED             ,
	WM_NOTIFICATION_RELEASED            ,
	WM_NOTIFICATION_MOVED_OUT           ,
	WM_NOTIFICATION_SEL_CHANGED         ,
	WM_NOTIFICATION_VALUE_CHANGED       ,
	WM_NOTIFICATION_SCROLLBAR_ADDED     ,
	WM_NOTIFICATION_CHILD_DELETED       ,
	WM_NOTIFICATION_GOT_FOCUS           ,
	WM_NOTIFICATION_LOST_FOCUS          ,
	WM_NOTIFICATION_SCROLL_CHANGED		,
	WM_NOTIFICATION_END
};
template<int code> constexpr int WM_NOTIFICATION_WIDGET = (static_cast<int>(WM_NOTIFICATION_END) + code);

#pragma endregion

#pragma region Window create flags
/**********************************************************************
 * These flags can be passed to the create window					  *
 * function as flag-parameter. The flags are combinable using the	  *
 * binary or operator.												  *
 **********************************************************************/
using WM_CF = uint16_t;
constexpr WM_CF
	 /* Hide window after creation (default !) */
	 WC_HIDE             = 0 << 0, 
	 /* Show window after creation */
	 WC_VISIBLE          = 1 << 0, 
	 /* Use memory device for redraws */
	 WC_MEMDEV           = 1 << 1, 
	 /* Stay on top */
	 WC_STAYONTOP        = 1 << 2, 
	 /* Disabled: Does not receive PID (mouse & touch) input */
	 WC_DISABLED         = 1 << 3, 

	 /* Create only flags ... Not available as status flags */
	
	 /* If automatic activation upon creation of window is desired */
	 WC_ACTIVATE         = 1 << 4, 
	 /* Put window in foreground after creation (default !) */
	 WC_FGND             = 0 << 5, 
	 /* Put window in background after creation */
	 WC_BGND             = 1 << 5, 

	 /* Anchor flags */
	
	 /* Right anchor ... If parent is resized, distance to right will remain const (left is default) */
	 WC_ANCHOR_RIGHT     = 1 << 6, 
	 /* Bottom anchor ... If parent is resized, distance to bottom will remain const (top is default) */
	 WC_ANCHOR_BOTTOM    = 1 << 7, 
	 /* Left anchor ... If parent is resized, distance to left will remain const (left is default) */
	 WC_ANCHOR_LEFT      = 1 << 8, 
	 /* Top anchor ... If parent is resized, distance to top will remain const (top is default) */
	 WC_ANCHOR_TOP       = 1 << 9,

	 /* Constant outline. This is relevant for transparent windows only. If a window is transparent and does not have a constant outline, its background is invalided instead of the window itself. This causes add. computation time when redrawing. */
	 WC_CONST_OUTLINE    = 1 << 10,
	 WC_LATE_CLIP        = 1 << 11,
	 WC_MEMDEV_ON_REDRAW = 1 << 12,

	 WC_RESERVED3        = 1 << 13,
	 WC_RESERVED4        = 1 << 14,
	 WC_RESERVED5        = 1 << 15;
#pragma endregion

/************************************************************
*
*                    GUI_KEY_...
*
*************************************************************
These ID values are basically meant to be used with widgets
Note that we have chosen the values to be close to existing
"standards", so do not change them unless forced to.
*/
constexpr int 
	GUI_KEY_BACKSPACE = 8 ,        /* ASCII: BACKSPACE Crtl-H */
	GUI_KEY_TAB       = 9 ,        /* ASCII: TAB       Crtl-I */
	GUI_KEY_ENTER     = 13,        /* ASCII: ENTER     Crtl-M */
	GUI_KEY_LEFT      = 16,
	GUI_KEY_UP        = 17,
	GUI_KEY_RIGHT     = 18,
	GUI_KEY_DOWN      = 19,
	GUI_KEY_HOME      = 23,
	GUI_KEY_END       = 24,
	GUI_KEY_SHIFT     = 25,
	GUI_KEY_CONTROL   = 26,
	GUI_KEY_ESCAPE    = 27,        /* ASCII: ESCAPE    0x1b   */
	GUI_KEY_INSERT    = 29,
	GUI_KEY_DELETE    = 30,
	GUI_KEY_SPACE     = 32,
	GUI_KEY_F1        = 40,
	GUI_KEY_F2        = 41;

/*********************************************************************
*
*       Dialog item IDs
*
*  The IDs below are arbitrary values. They just have to be unique
*  within the dialog.
*
*  If you use your own Ids, we recommend to use values above GUI_ID_USER.
*/
constexpr uint16_t
	GUI_ID_OK        = 1,
	GUI_ID_CANCEL    = 2,
	GUI_ID_YES       = 3,
	GUI_ID_NO        = 4,
	GUI_ID_CLOSE     = 5,
	GUI_ID_HELP      = 6,
	GUI_ID_MAXIMIZE  = 7,
	GUI_ID_MINIMIZE  = 8,
	GUI_ID_VSCROLL   = 0xFE,
	GUI_ID_HSCROLL   = 0xFF,
	GUI_ID_USER      = 0x100;


#pragma region Windows parameter type
using WM_PARAM = uintptr_t;

struct WM_KEY_INFO {
	int16_t Key, PressedCnt;
};
struct WM_SCROLL_STATE {
	int16_t NumItems = 0, PageSize = 0, v = 0;
	void Bounds() {
		int Max = NumItems - PageSize;
		if (Max < 0)
			Max = 0;
		/* Make sure scroll pos is in bounds */
		if (v < 0)
			v = 0;
		if (v > Max)
			v = Max;
	}
	auto CheckPos(int Pos, int LowerDist, int UpperDist) {
		int vOld = v;
		/* Check upper limit */
		if (Pos > v + PageSize - 1)
			v = Pos - (PageSize - 1) + UpperDist;
		/* Check lower limit */
		if (Pos < v)
			v = Pos - LowerDist;
		Bounds();
		return v - vOld;
	}
	auto SetValue(int v) {
		int vOld = this->v;
		this->v = v;
		Bounds();
		return this->v - vOld;
	}
	inline bool operator!=(const WM_SCROLL_STATE &other) const {
		return NumItems != other.NumItems || v != other.v || PageSize != other.PageSize;
	}
	inline bool operator==(const WM_SCROLL_STATE &other) const {
		return !(*this != other);
	}
};
struct DIALOG_STATUS {
	int16_t Done = 0, ReturnValue = 0;
};

struct PID_CHANGED_INFO {
	int16_t x, y;
	uint8_t State, StatePrev;
};
#pragma endregion

}
