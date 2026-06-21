

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"

#define GUI_TIMER_H2P(h) (GUI_TIMER_Obj*)GUI_ALLOC_h2p(h)

typedef struct {
  GUI_TIMER_CALLBACK* cb;
  GUI_TIMER_HANDLE hNext;
  int Flags;
	uint32_t Context;
	GUI_TIMER_TIME t0;
	GUI_TIMER_TIME Period;
} GUI_TIMER_Obj;

GUI_TIMER_HANDLE hFirstTimer;
GUI_TIMER_HANDLE _hActiveTimer;

static void _Unlink(GUI_TIMER_HANDLE hTimer) {
  GUI_TIMER_Obj* pTimer = GUI_TIMER_H2P(hTimer);
  GUI_TIMER_HANDLE hi;
  GUI_TIMER_Obj*   pi;
/* Check if it is the first element */
  if (hFirstTimer == hTimer) {
    hFirstTimer = pTimer->hNext;
    return;
	}
  hi = hFirstTimer;
/* Try to find it in the list ... */
  while(hi) {
    /* GUI_ASSERT(hi<1000,0); */
    pi = GUI_TIMER_H2P(hi);
    if (pi->hNext == hTimer) {
      pi->hNext = pTimer->hNext;
      break;
		}
    hi = pi->hNext;
  }
}

/*********************************************************************
*
*       _Link
*
* Purpose:
*   This routine inserts the new timer (referenced by its handle) into
*	  the linked list. The linked list is sorted according to timestamps.
*	  The first element is the timer which expires first.
*/
static void _Link(GUI_TIMER_HANDLE hNew) {
  GUI_TIMER_Obj*   pNew        = GUI_TIMER_H2P(hNew);
  GUI_TIMER_Obj*   pTimer;
  GUI_TIMER_Obj*   pNext;
  GUI_TIMER_HANDLE hNext;
  if (hFirstTimer ==0) { /* List is empty, make it the only element */
    hFirstTimer = hNew;
	  pNew->hNext = 0;
  } else {
    GUI_TIMER_Obj* pFirstTimer      = GUI_TIMER_H2P(hFirstTimer);
/* Check if we have to make it the first element */
    if ((pNew->t0 - pFirstTimer->t0) <=0) {
      pNew->hNext = hFirstTimer;
      hFirstTimer = hNew;
			return;
		} else {
      GUI_TIMER_HANDLE hTimer = hFirstTimer;
/* Put it into the list */
      do {
        pTimer       = GUI_TIMER_H2P(hTimer);
        hNext        = pTimer->hNext;
        if (hNext ==0)
					goto Append;
        pNext      = GUI_TIMER_H2P(hNext);
				if ((pNew->t0 - pNext->t0) <=0) {
          pNew->hNext  = hNext;
          pTimer->hNext= hNew;
          return;
				}
			} while(1);
/* Put it at the end of the list */
Append:
      pNew->hNext  = hNext;
      pTimer->hNext= hNew;
      return;
		}
  }
}

int GUI_TIMER_Exec(void) {
  int r = 0;
  GUI_TIMER_TIME t = GUI_GetTime();
   {
    while (hFirstTimer) {
     	GUI_TIMER_Obj* pTimer = GUI_TIMER_H2P(hFirstTimer);
      if ((pTimer->t0-t) <=0) {
        GUI_TIMER_MESSAGE tm;
        tm.Time = t;
				tm.Context = pTimer->Context;
        _hActiveTimer = hFirstTimer;
        hFirstTimer = pTimer->hNext;
        pTimer->cb(&tm);
        r = 1;
			} else
			  break;
    }
    /*
		GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->t0 = Time;
    */
  }
  return r;
}

GUI_TIMER_HANDLE GUI_TIMER_Create(GUI_TIMER_CALLBACK* cb, int Time, uint32_t Context, int Flags) {
  GUI_TIMER_HANDLE hObj;
  GUI_TIMER_Obj* pObj;

  GUI_USE_PARA(Flags);
  GUI_USE_PARA(Time);
  GUI_pfTimerExec = GUI_TIMER_Exec;
	{
    /* Alloc memory for obj */
    hObj = GUI_ALLOC_AllocZero(sizeof(GUI_TIMER_Obj));
    pObj = GUI_TIMER_H2P(hObj);
    /* init member variables */
    pObj->cb = cb;
		pObj->Context = Context;
		pObj->t0 = Time;	//houhh 20061018...
    /* Link it */
		_Link(hObj);
	}
  return hObj;
}

void GUI_TIMER_Delete(GUI_TIMER_HANDLE hObj) {
/* Unlink Timer */

  _Unlink(hObj);
  GUI_ALLOC_Free(hObj);

}

void GUI_TIMER_SetPeriod(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Period) {
   {
    GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->Period = Period;
  }
}

void GUI_TIMER_SetTime(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Time) {
   {
   	GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->t0 = Time;
  }
}

//////
void GUI_TIMER_Context(GUI_TIMER_HANDLE hObj, uint32_t Context) {
   {
   	GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->Context = Context;
  }
}
//////

void GUI_TIMER_SetDelay(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Delay) {
   {
   	GUI_TIMER_Obj* pObj = GUI_TIMER_H2P(hObj);
    pObj->t0 = Delay;
		_Unlink(hObj);
		_Link(hObj);
  }
}

void GUI_TIMER_Restart(GUI_TIMER_HANDLE hObj) {
  GUI_TIMER_Obj* pObj;

  {
    if (hObj == 0) {
      hObj = _hActiveTimer;
    }
   	pObj = GUI_TIMER_H2P(hObj);
    pObj->t0 = GUI_GetTime() +pObj->Period;
		_Unlink(hObj);
		_Link(hObj);
  }
}

/*************************** End of file ****************************/
