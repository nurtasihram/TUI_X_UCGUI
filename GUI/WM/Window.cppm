export module TUX.Window;

export import TUX.Types;
export import TUX.WindowTypes;

export {
	
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

public:
	auto GetRect() const { return Rect; }
	
	auto GetOrg() const { return Rect.LeftTop(); }
	auto GetOrgX() const { return Rect.x0; }
	auto GetOrgY() const { return Rect.y0; }

	auto GetSize() const { return Rect.Size(); }
	auto GetSizeX() const { return Rect.XSize(); }
	auto GetSizeY() const { return Rect.YSize(); }

};


}
