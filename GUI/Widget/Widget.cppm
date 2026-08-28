module;

#include <memory>

#include "WM_Intern.h"
#include "GUI_Protected.h"

export module TUX.Widget;

export import TUX.Window;
export import TUX.WindowTypes;

export {

constexpr uint16_t
	WIDGET_STATE_FOCUS       = 1 << 0,
	WIDGET_STATE_FOCUSSABLE  = 1 << 1,
	WIDGET_STATE_USER_START  = 1 << 2;
template<uint8_t bits>
constexpr uint16_t
	WIDGET_STATE_USER = 1 << (WIDGET_STATE_USER_START + bits);

constexpr uint16_t WM_WIDGET_SET_EFFECT     = WM_WIDGET<0>;

constexpr uint8_t
	WIDGET_ITEM_DRAW          = 0,
	WIDGET_ITEM_GET_XSIZE     = 1,
	WIDGET_ITEM_GET_YSIZE     = 2;
typedef int WIDGET_DRAW_ITEM_FUNC(WObj *pWin, int Cmd, int ItemIndex, POINT ItemPos);

#pragma region Widget Effect
struct WIDGET_EFFECT {
	void DrawUp(void) const { DrawUp(WM_GetClientRect()); }
	void DrawDown(void) const { DrawDown(WM_GetClientRect()); }
	RECT GetRect() const { return WM_GetClientRect() / EffectSize; }

	virtual void DrawUp(RECT r) const {}
	virtual void DrawDown(RECT r) const {}

	int EffectSize;

	WIDGET_EFFECT(int EffectSize = 0) : EffectSize(EffectSize) {}
};
using CWIDGET_EFFECT = const WIDGET_EFFECT;
using PCWIDGET_EFFECT = const WIDGET_EFFECT *;

extern PCWIDGET_EFFECT
	WIDGET_Effect_None,
	WIDGET_Effect_Simple,
	WIDGET_Effect_3D,
	WIDGET_Effect_3D1L,
	WIDGET_Effect_3D2L;
#pragma endregion

#pragma region Draw
typedef void GUI_DRAW_SELF_CB(RECT &r);
struct GUI_DRAW {
	void Draw(RECT r) const {
		r += GUI.Off;
		Paint(r);
	}
	virtual void Paint(RECT &r) const = 0;
	virtual int GetXSize() const { return 0; }
	virtual int GetYSize() const { return 0; }
	GUI_DRAW() {}
};

GUI_DRAW *GUI_DRAW_BITMAP_Create(PCBITMAP pBitmap) {
	struct _GUI_DRAW : public GUI_DRAW {
		PCBITMAP pBitmap;
		_GUI_DRAW(PCBITMAP pBitmap) : GUI_DRAW(), pBitmap(pBitmap) {}
		void Paint(RECT &r) const override {
			GUI_DrawBitmap(pBitmap, r.x0, r.y0);
		}
		int GetXSize() const override {
			return pBitmap ? pBitmap->XSize : 0;
		}
		int GetYSize() const override {
			return pBitmap ? pBitmap->YSize : 0;
		}
	};
	auto pObj = (GUI_DRAW *)GUI_ALLOC_AllocZero(sizeof(_GUI_DRAW));
	if (pObj)
		new (pObj) _GUI_DRAW{ pBitmap };
	return pObj;
}

GUI_DRAW *GUI_DRAW_SELF_Create(GUI_DRAW_SELF_CB *pfDraw) {
	struct _GUI_DRAW : public GUI_DRAW {
		GUI_DRAW_SELF_CB *pfDraw;
		_GUI_DRAW(GUI_DRAW_SELF_CB *pfDraw) : GUI_DRAW(), pfDraw(pfDraw) {}
		void Paint(RECT &r) const override {
			if (pfDraw)
				pfDraw(r);
		}
	};
	auto pObj = (GUI_DRAW *)GUI_ALLOC_AllocZero(sizeof(_GUI_DRAW));
	if (pObj)
		new (pObj) _GUI_DRAW{ pfDraw };
	return pObj;
}
#pragma endregion

class Widget : public WObj {

public:
	static PCWIDGET_EFFECT DefaultEffect;

private:
	PCWIDGET_EFFECT pEffect = DefaultEffect;
	uint16_t Id, State;

public:
	Widget(RECT r, WM_CF Style, WM_CALLBACK *cb, WObj *pParent, uint16_t Id, uint16_t State) :
		WObj(r, Style, cb, pParent), Id(Id), State(State) {}

public:
	auto GetEffect() const { return pEffect; }
	void SetEffect(const WIDGET_EFFECT *pEffect)
	{ Require(WM_WIDGET_SET_EFFECT, (WM_PARAM)pEffect); }
	auto EffectSize() const { return pEffect ? pEffect->EffectSize : 0; }

	uint16_t GetId() const { return Id; }

protected:
	void SetBkColorPrefer(RGBC BkColor) {
		while (BkColor == RGB_INVALID_COLOR) {
			if (auto pParent = Parent())
				BkColor = pParent->GetBkColor();
			else
				break;
		}
		GUI.SetBkColor(BkColor);
	}

	void SetScrollState(const WM_SCROLL_STATE &VState, const WM_SCROLL_STATE &HState);

	void DrawUp() const {
		if (pEffect)
			pEffect->DrawUp();
	}
	void DrawDown() const {
		if (pEffect)
			pEffect->DrawDown();
	}
	void DrawUp(const RECT &r) const {
		if (pEffect)
			pEffect->DrawUp(r);
	}
	void DrawDown(const RECT &r) const {
		if (pEffect)
			pEffect->DrawDown(r);
	}

	auto GetStates() const { return State; }
	bool SetStates(uint16_t States) {
		if (State == States)
			return false;
		State = States;
		Invalidate();
		return true;
	}
	bool AddStates(uint16_t States)
	{ return SetStates(State | States); }
	bool DelStates(uint16_t States)
	{ return SetStates(State & ~States); }
	bool CtlStates(uint16_t States, bool On)
	{ return SetStates(On ? State | States : State & ~States); }
	
	RECT _GetInsideRect() { return GetClientRect() / EffectSize(); }

	bool HandleActive(int MsgId, WM_PARAM *Data) {
		switch (MsgId) {
			case WM_WIDGET_SET_EFFECT: {
				auto Diff = EffectSize();
				pEffect = (const WIDGET_EFFECT *)*Data;
				Diff -= EffectSize();
				if (Diff)
					_UpdateChildPositions({ -Diff, Diff });
				Invalidate();
				return false; /* Message handled -> Return */
			}
			case WM_GET_ID:
				*Data = Id;
				return false; /* Message handled -> Return */
			case WM_PID_STATE_CHANGED:
				if (State & WIDGET_STATE_FOCUSSABLE) {
					auto pInfo = (const PID_CHANGED_INFO *)*Data;
					if (pInfo->State)
						SetFocus();
				}
				break;
			case WM_TOUCH_CHILD: {
				/* A descendent (child) has been touched or released.
				   If it has been touched, we need to get to top.
				 */
				auto pState = (const PID_STATE *)*Data;
				if (pState) { /* Message may not have a valid pointer (moved out) ! */
					if (pState->Pressed) {
						BringToTop();
						return false; /* Message handled -> Return */
					}
				}
				break;
			}
			case WM_SET_ID:
				Id = (int16_t)*Data;
				return false; /* Message handled -> Return */
			case WM_SET_FOCUS: {
				int Notification;
				if (*Data) {
					SetStates(State | WIDGET_STATE_FOCUS);
					Notification = WM_NOTIFICATION_GOT_FOCUS;
				}
				else {
					SetStates(State & ~WIDGET_STATE_FOCUS);
					Notification = WM_NOTIFICATION_LOST_FOCUS;
				}
				NotifyParent(Notification);
				*Data = 0;   /* Focus change accepted */
				return false;
			}
			case WM_GET_ACCEPT_FOCUS:
				*(bool *)Data = State & WIDGET_STATE_FOCUSSABLE; /* Can handle focus */
				return false; /* Message handled */
			case WM_GET_INSIDE_RECT:
				*(RECT *)*Data = _GetInsideRect();
				return false; /* Message handled */
		}
		return true; /* Message NOT handled */
	}

public:

	struct CreateStruct {

		Widget* (*pfCreateIndirect)(const CreateStruct* pCreate, WObj* hWin, int x0, int y0, WM_CALLBACK* cb);

		const char* pName; /* Text ... Not used on all widgets */
		int16_t Id; /* ID ... should be unique in a dialog */
		int16_t x0, y0, xSize, ySize; /* Define position and size */
		uint16_t Flags; /* Widget specific create flags (opt.) */
		int32_t Para; /* Widget specific parameter (opt.) */

		WObj* CreateDialog(int NumWidgets, WM_CALLBACK* cb, WObj* hParent, int x0, int y0) const {
			auto pDialog = pfCreateIndirect(this, hParent, x0, y0, cb);     /* Create parent window */
			auto pDialogClient = pDialog->Client();
			pDialog->AddStates(Flags);
			pDialog->ShowWindow();
			pDialogClient->ShowWindow();
			auto paWidget = this;
			while (--NumWidgets > 0) {
				paWidget++;
				auto pChild = paWidget->pfCreateIndirect(paWidget, pDialogClient, 0, 0, 0); /* Create child window */
				pChild->ShowWindow();
			}
			pDialog->SetFocusOnNextChild();     /* Set the focus to the first child */
			pDialogClient->Require(WM_INIT_DIALOG);
			return pDialog;
		}

		int ExecDialog(int NumWidgets, WM_CALLBACK* cb, WObj* pParent, int x0, int y0) const {
			auto pDialog = CreateDialog(NumWidgets, cb, pParent, x0, y0);
			return pDialog->DialogExec();
		}

	};

};

void WIDGET__FillStringInRect(const char *pText, RECT FillRect, RECT TextRectMax, RECT TextRectAct) {
	/* Check if we have anything to do at all ... */
	auto r = FillRect + GUI.Off;
	if (!(r <= GUI.ClipRect))
		return;
	/* Fill border */
	GUI_ClearRect(FillRect);
	if (!pText) return;
	if (!*pText) return;
	/* Set clipping rectangle */
	auto pOldClipRect = WM_SetUserClipRect(&TextRectMax);
	/* Display text */
	GUI.SetTextMode(DRAWMODE_NORMAL);
	GUI_DispStringAt(pText, TextRectAct.x0, TextRectAct.y0);
	/* Restore clipping rectangle */
	WM_SetUserClipRect(pOldClipRect);
}

}

PCWIDGET_EFFECT Widget::DefaultEffect = WIDGET_Effect_3D2L;
