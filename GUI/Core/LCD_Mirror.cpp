

#include "GUI.h"
#include "LCD_Protected.h"    /* private modul definitions & config */

const uint8_t LCD_aMirror[256] = {
  ________________, XX______________, __XX____________, XXXX____________, ____XX__________, XX__XX__________, __XXXX__________, XXXXXX__________,
  ______XX________, XX____XX________, __XX__XX________, XXXX__XX________, ____XXXX________, XX__XXXX________, __XXXXXX________, XXXXXXXX________,
  ________XX______, XX______XX______, __XX____XX______, XXXX____XX______, ____XX__XX______, XX__XX__XX______, __XXXX__XX______, XXXXXX__XX______,
  ______XXXX______, XX____XXXX______, __XX__XXXX______, XXXX__XXXX______, ____XXXXXX______, XX__XXXXXX______, __XXXXXXXX______, XXXXXXXXXX______,
  __________XX____, XX________XX____, __XX______XX____, XXXX______XX____, ____XX____XX____, XX__XX____XX____, __XXXX____XX____, XXXXXX____XX____,
  ______XX__XX____, XX____XX__XX____, __XX__XX__XX____, XXXX__XX__XX____, ____XXXX__XX____, XX__XXXX__XX____, __XXXXXX__XX____, XXXXXXXX__XX____,
  ________XXXX____, XX______XXXX____, __XX____XXXX____, XXXX____XXXX____, ____XX__XXXX____, XX__XX__XXXX____, __XXXX__XXXX____, XXXXXX__XXXX____,
  ______XXXXXX____, XX____XXXXXX____, __XX__XXXXXX____, XXXX__XXXXXX____, ____XXXXXXXX____, XX__XXXXXXXX____, __XXXXXXXXXX____, XXXXXXXXXXXX____,
  ____________XX__, XX__________XX__, __XX________XX__, XXXX________XX__, ____XX______XX__, XX__XX______XX__, __XXXX______XX__, XXXXXX______XX__,
  ______XX____XX__, XX____XX____XX__, __XX__XX____XX__, XXXX__XX____XX__, ____XXXX____XX__, XX__XXXX____XX__, __XXXXXX____XX__, XXXXXXXX____XX__,
  ________XX__XX__, XX______XX__XX__, __XX____XX__XX__, XXXX____XX__XX__, ____XX__XX__XX__, XX__XX__XX__XX__, __XXXX__XX__XX__, XXXXXX__XX__XX__,
  ______XXXX__XX__, XX____XXXX__XX__, __XX__XXXX__XX__, XXXX__XXXX__XX__, ____XXXXXX__XX__, XX__XXXXXX__XX__, __XXXXXXXX__XX__, XXXXXXXXXX__XX__,
  __________XXXX__, XX________XXXX__, __XX______XXXX__, XXXX______XXXX__, ____XX____XXXX__, XX__XX____XXXX__, __XXXX____XXXX__, XXXXXX____XXXX__,
  ______XX__XXXX__, XX____XX__XXXX__, __XX__XX__XXXX__, XXXX__XX__XXXX__, ____XXXX__XXXX__, XX__XXXX__XXXX__, __XXXXXX__XXXX__, XXXXXXXX__XXXX__,
  ________XXXXXX__, XX______XXXXXX__, __XX____XXXXXX__, XXXX____XXXXXX__, ____XX__XXXXXX__, XX__XX__XXXXXX__, __XXXX__XXXXXX__, XXXXXX__XXXXXX__,
  ______XXXXXXXX__, XX____XXXXXXXX__, __XX__XXXXXXXX__, XXXX__XXXXXXXX__, ____XXXXXXXXXX__, XX__XXXXXXXXXX__, __XXXXXXXXXXXX__, XXXXXXXXXXXXXX__,
  ______________XX, XX____________XX, __XX__________XX, XXXX__________XX, ____XX________XX, XX__XX________XX, __XXXX________XX, XXXXXX________XX,
  ______XX______XX, XX____XX______XX, __XX__XX______XX, XXXX__XX______XX, ____XXXX______XX, XX__XXXX______XX, __XXXXXX______XX, XXXXXXXX______XX,
  ________XX____XX, XX______XX____XX, __XX____XX____XX, XXXX____XX____XX, ____XX__XX____XX, XX__XX__XX____XX, __XXXX__XX____XX, XXXXXX__XX____XX,
  ______XXXX____XX, XX____XXXX____XX, __XX__XXXX____XX, XXXX__XXXX____XX, ____XXXXXX____XX, XX__XXXXXX____XX, __XXXXXXXX____XX, XXXXXXXXXX____XX,
  __________XX__XX, XX________XX__XX, __XX______XX__XX, XXXX______XX__XX, ____XX____XX__XX, XX__XX____XX__XX, __XXXX____XX__XX, XXXXXX____XX__XX,
  ______XX__XX__XX, XX____XX__XX__XX, __XX__XX__XX__XX, XXXX__XX__XX__XX, ____XXXX__XX__XX, XX__XXXX__XX__XX, __XXXXXX__XX__XX, XXXXXXXX__XX__XX,
  ________XXXX__XX, XX______XXXX__XX, __XX____XXXX__XX, XXXX____XXXX__XX, ____XX__XXXX__XX, XX__XX__XXXX__XX, __XXXX__XXXX__XX, XXXXXX__XXXX__XX,
  ______XXXXXX__XX, XX____XXXXXX__XX, __XX__XXXXXX__XX, XXXX__XXXXXX__XX, ____XXXXXXXX__XX, XX__XXXXXXXX__XX, __XXXXXXXXXX__XX, XXXXXXXXXXXX__XX,
  ____________XXXX, XX__________XXXX, __XX________XXXX, XXXX________XXXX, ____XX______XXXX, XX__XX______XXXX, __XXXX______XXXX, XXXXXX______XXXX,
  ______XX____XXXX, XX____XX____XXXX, __XX__XX____XXXX, XXXX__XX____XXXX, ____XXXX____XXXX, XX__XXXX____XXXX, __XXXXXX____XXXX, XXXXXXXX____XXXX,
  ________XX__XXXX, XX______XX__XXXX, __XX____XX__XXXX, XXXX____XX__XXXX, ____XX__XX__XXXX, XX__XX__XX__XXXX, __XXXX__XX__XXXX, XXXXXX__XX__XXXX,
  ______XXXX__XXXX, XX____XXXX__XXXX, __XX__XXXX__XXXX, XXXX__XXXX__XXXX, ____XXXXXX__XXXX, XX__XXXXXX__XXXX, __XXXXXXXX__XXXX, XXXXXXXXXX__XXXX,
  __________XXXXXX, XX________XXXXXX, __XX______XXXXXX, XXXX______XXXXXX, ____XX____XXXXXX, XX__XX____XXXXXX, __XXXX____XXXXXX, XXXXXX____XXXXXX,
  ______XX__XXXXXX, XX____XX__XXXXXX, __XX__XX__XXXXXX, XXXX__XX__XXXXXX, ____XXXX__XXXXXX, XX__XXXX__XXXXXX, __XXXXXX__XXXXXX, XXXXXXXX__XXXXXX,
  ________XXXXXXXX, XX______XXXXXXXX, __XX____XXXXXXXX, XXXX____XXXXXXXX, ____XX__XXXXXXXX, XX__XX__XXXXXXXX, __XXXX__XXXXXXXX, XXXXXX__XXXXXXXX,
  ______XXXXXXXXXX, XX____XXXXXXXXXX, __XX__XXXXXXXXXX, XXXX__XXXXXXXXXX, ____XXXXXXXXXXXX, XX__XXXXXXXXXXXX, __XXXXXXXXXXXXXX, XXXXXXXXXXXXXXXX
};
