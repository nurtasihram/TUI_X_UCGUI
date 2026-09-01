module;

#include "GUI.h"

export module TUX.Core.Timer;

import TUX;

export {

struct GUI_TIMER_MESSAGE {
	GUI_TIMER_TIME Time;
	uintptr_t Context;
};
typedef void GUI_TIMER_CALLBACK(/*const */ GUI_TIMER_MESSAGE *pTM);

class Timer {
	GUI_TIMER_CALLBACK *cb = nullptr;
	Timer *pNext = nullptr;
	uintptr_t Context = 0;
	GUI_TIMER_TIME t0 = 0, Period = 0;

	static Timer *pFirstTimer;
	void _Link() {
		pNext = nullptr;
		if (!pFirstTimer) { /* List is empty, make it the only element */
			pFirstTimer = this;
			return;
		}
		/* Check if we have to make it the first element */
		if (t0 <= pFirstTimer->t0) {
			pNext = pFirstTimer;
			pFirstTimer = this;
			return;
		}
		auto pPrev = pFirstTimer;
		/* Put it into the list */
		while (auto pNextTimer = pPrev->pNext) {
			if (t0 <= pNextTimer->t0) {
				pNext = pNextTimer;
				pPrev->pNext = this;
				return;
			}
			pPrev = pNextTimer;
		}
		pPrev->pNext = this;
	}
	void _Unlink() {
		/* Check if it is the first element */
		if (pFirstTimer == this) {
			pFirstTimer = pNext;
			return;
		}
		/* Try to find it in the list ... */
		for (auto pi = pFirstTimer; pi; pi = pi->pNext)
			if (pi->pNext == this) {
				pi->pNext = pNext;
				break;
			}
	}

public:

	Timer(GUI_TIMER_CALLBACK *cb, GUI_TIMER_TIME Time, uint32_t Context = 0) :
		cb(cb), Context(Context), t0(Time) { _Link(); }

	~Timer() { _Unlink(); }

	void *operator new(size_t Size) { return GUI_ALLOC_Alloc(Size); }
	void operator delete(void *p) { GUI_ALLOC_Free(p); }

public:

	void SetTime(GUI_TIMER_TIME Time) {
		t0 = Time;
		_Unlink();
		_Link();
	}
	void SetPeriod(GUI_TIMER_TIME Period) { this->Period = Period; }
	void SetContext(auto Context) requires(sizeof(Context) <= sizeof(uintptr_t))
	{ this->Context = (uintptr_t)Context; }

	void Restart() {
		t0 = GUI_GetTime() + Period;
		_Unlink();
		_Link();
	}

	static bool Exec() {
		static Timer *pActiveTimer = nullptr;
		bool r = false;
		auto t = GUI_GetTime();
		while (auto pTimer = pFirstTimer) {
			if (pTimer->t0 > t)
				break;
			GUI_TIMER_MESSAGE tm;
			tm.Time = t;
			tm.Context = pTimer->Context;
			pActiveTimer = pTimer;
			pFirstTimer = pTimer->pNext;
			pTimer->cb(&tm);
			r = true;
		}
		return r;
	}

};

Timer *Timer::pFirstTimer = nullptr;

}
