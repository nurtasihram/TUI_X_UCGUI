

#include "GUI_Protected.h"
#include "math.h"

static void _DispFloatFix(float f, char Len, char Decs, int DrawPlusSign) {
  f *= GUI_Pow10[(unsigned)Decs];
  f += 0.5;
  f = (float) floor (f);
  if (DrawPlusSign) {
    GUI_DispSDecShift((long)f, Len, Decs);
  } else {
    GUI_DispDecShift((long)f, Len, Decs);
  }
}

void GUI_DispFloatFix(float f, char Len, char Decs) {
  _DispFloatFix(f, Len, Decs, 0);
}

void GUI_DispFloatMin(float f, char Fract) {
  char Len;
  Len = GUI_Long2Len((long)f);
  if ((f < 0) && (f > -1)) { /* If value < 0 and > -1 (e.g. -0.123) increment length by 1 */
    Len++;
  }
  _DispFloatFix(f, (char)(Len + Fract + (Fract ? 1 : 0)), (char)Fract, 0);
}

void GUI_DispFloat(float f, char Len) {
  int Decs;
  Decs =  Len - GUI_Long2Len((long)f)-1;
  if ((f < 0) && (f > -1)) { /* If value < 0 and > -1 (e.g. -0.123) decrement Decs */
    Decs--;
  }
  if (Decs<0)
    Decs =0;
  _DispFloatFix(f, Len, (char)Decs, 0);
}

void GUI_DispSFloatFix(float f, char Len, char Fract) {
  _DispFloatFix (f, Len, Fract, 1);
}

void GUI_DispSFloatMin(float f, char Fract) {
  char Len;
  Len = GUI_Long2Len((long)f);
  if ((f < 0) && (f > -1)) { /* If value < 0 and > -1 (e.g. -0.123) increment length by 1 */
    Len++;
  }
  if (f>0) {
    Len++;
  }
  _DispFloatFix(f, (char)(Len + Fract + (Fract ? 1 : 0)), (char)Fract, 1);
}

/*************************** End of file ****************************/
