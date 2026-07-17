#include "GUI.h"

/*      *********************************
        *                               *
        *   Special character codes     *
        *                               *
        *********************************
*/

/* No 95 to 125 for European character set  */

#define CODE_SACCAGUE     95     /*  small accent ague */
#define CODE_SACCGRAV     96
#define CODE_SACCCIRC     97
#define CODE_SUMLAUT      98
#define CODE_STILDE       99
#define CODE_I_NOPOINT    100

#define CODE_SHARPS       101
#define CODE_A_RING       102
#define CODE_SA_RING      103
#define CODE_AE           104
#define CODE_ETH          105
#define CODE_THORN        106
#define CODE_SMALLAE      107
#define CODE_SMALLETH     108
#define CODE_SMALLTHORN   109
#define CODE_OSLASH       110
#define CODE_SOSLASH      111
#define CODE_LITTLE_A     112
#define CODE_LITTLE_E     113
#define CODE_LITTLE_I     114
#define CODE_LITTLE_O     115
#define CODE_LITTLE_U     116
#define CODE_LITTLE_N     117

#define CODE_INVEXCLAM    118
#define CODE_INVQUEST     119

#define CODE_CACCAGUE     120    /* capital accent ague */
#define CODE_CACCGRAV     121
#define CODE_CACCCIRC     122
#define CODE_CUMLAUT      123
#define CODE_CTILDE       124
#define CODE_CEDILLA      125

/* No 126 to 156 for complete ISO 8859_1 western latin character set  */
#define CODE_NB_SPACE     126
#define CODE_CENT         127
#define CODE_POUND        128
#define CODE_CURRENCY     129
#define CODE_YEN          130
#define CODE_BROKEN_BAR   131
#define CODE_SECTION      132
#define CODE_DIERESIS     133
#define CODE_COPYRIGHT    134
#define CODE_FEMININE     135
#define CODE_LEFT_QUOTE   136
#define CODE_NOT          137
#define CODE_HYPHEN       138
#define CODE_TRADEMARK    139
#define CODE_MACRON       140
#define CODE_DEGREE       141
#define CODE_PLUS_MINUS   142
#define CODE_SUPER_TWO    143
#define CODE_SUPER_THREE  144
#define CODE_ACUTE        145
#define CODE_MICRO        146
#define CODE_PARAGRAPH    147
#define CODE_MIDDLE_DOT   148
#define CODE_SUPER_ONE    149
#define CODE_MASCULINE    150
#define CODE_RIGHT_QUOTE  151
#define CODE_ONE_FOURTH   152
#define CODE_ONE_HALF     153
#define CODE_THREE_FOURTH 154
#define CODE_MULTIPLY     155
#define CODE_DIVISION     156
/* The following are extensions to ISO 8859-1 in the area which is not
   defined by the standard.
*/
#define CODE_ARROW_LEFT   157
#define CODE_ARROW_RIGHT  158
#define CODE_ARROW_UP     159
#define CODE_ARROW_DOWN   160
#define CODE_ENTER        161
#define CODE_CHECKMARK    162





/*
    ****************************************************************
    *                                                              *
    *                      8 * 8  font                             *
    *                                                              *
    ****************************************************************
*/


const uint8_t GUI_F8x8_acFont[][8] = {

  {
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}  /* char ' '  */

 ,{
   ______XXXX______,
   ____XXXXXXXX____,
   ____XXXXXXXX____,
   ______XXXX______,
   ______XXXX______,
   ________________,
   ______XXXX______,
   ________________}  /* char '!'  */

 ,{
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   ____XX____XX____,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}  /* char '"'  */

 ,{
   __XXXX__XXXX____,
   __XXXX__XXXX____,
   XXXXXXXXXXXXXX__,
   __XXXX__XXXX____,
   XXXXXXXXXXXXXX__,
   __XXXX__XXXX____,
   __XXXX__XXXX____,
   ________________}  /* char '#'  */

 ,{
   ______XXXX______,
   ____XXXXXXXXXX__,
   __XXXX__________,
   ____XXXXXXXX____,
   __________XXXX__,
   __XXXXXXXXXX____,
   ______XXXX______,
   ________________}  /* char '$'  */

 ,{
   ________________,
   XXXX______XXXX__,
   XXXX____XXXX____,
   ______XXXX______,
   ____XXXX________,
   __XXXX____XXXX__,
   XXXX______XXXX__,
   ________________}  /* char '%'  */

 ,{
   ____XXXXXX______,
   __XXXX__XXXX____,
   ____XXXXXX______,
   __XXXXXX__XXXX__,
   XXXX__XXXXXX____,
   XXXX____XXXX____,
   __XXXXXX__XXXX__,
   ________________}  /* char '&'  */

 ,{
   ______XXXX______,
   ______XXXX______,
   ____XXXX________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}  /* char '''  */

 ,{
   ________XXXX____,
   ______XXXX______,
   ____XXXX________,
   ____XXXX________,
   ____XXXX________,
   ______XXXX______,
   ________XXXX____,
   ________________}  /* char '('  */

 ,{
   ____XXXX________,
   ______XXXX______,
   ________XXXX____,
   ________XXXX____,
   ________XXXX____,
   ______XXXX______,
   ____XXXX________,
   ________________}  /* char ')'  */

 ,{
   ________________,
   __XXXX____XXXX__,
   ____XXXXXXXX____,
   XXXXXXXXXXXXXXXX,
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   ________________,
   ________________}  /* char '*'  */

 ,{
   ________________,
   ______XXXX______,
   ______XXXX______,
   __XXXXXXXXXXXX__,
   ______XXXX______,
   ______XXXX______,
   ________________,
   ________________}  /* char '+'  */

 ,{
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ______XXXX______,
   ______XXXX______,
   ____XXXX________}  /* char ','  */

 ,{
   ________________,
   ________________,
   ________________,
   __XXXXXXXXXXXX__,
   ________________,
   ________________,
   ________________,
   ________________}  /* char '-'  */

 ,{
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ______XXXX______,
   ______XXXX______,
   ________________}  /* char '.'  */

 ,{
   __________XXXX__,
   ________XXXX____,
   ______XXXX______,
   ____XXXX________,
   __XXXX__________,
   XXXX____________,
   XX______________,
   ________________}  /* char '/'  */

 ,{
   ____XXXXXX______,
   __XXXX__XXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXX__XXXX____,
   ____XXXXXX______,
   ________________}  /* char '0'  */

 ,{
   ______XXXX______,
   ____XXXXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   __XXXXXXXXXXXX__,
   ________________}  /* char '1'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   __________XXXX__,
   ______XXXXXX____,
   ____XXXX________,
   __XXXX____XXXX__,
   XXXXXXXXXXXXXX__,
   ________________}  /* char '2'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   __________XXXX__,
   ____XXXXXXXX____,
   __________XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char '3'  */

 ,{
   ______XXXXXX____,
   ____XXXXXXXX____,
   __XXXX__XXXX____,
   XXXX____XXXX____,
   XXXXXXXXXXXXXX__,
   ________XXXX____,
   ______XXXXXXXX__,
   ________________}  /* char '4'  */

 ,{
   XXXXXXXXXXXXXX__,
   XXXX____________,
   XXXX____________,
   XXXXXXXXXXXX____,
   __________XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char '5'  */

 ,{
   ____XXXXXX______,
   __XXXX__________,
   XXXX____________,
   XXXXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char '6'  */

 ,{
   XXXXXXXXXXXXXX__,
   XXXX______XXXX__,
   ________XXXX____,
   ______XXXX______,
   ____XXXX________,
   ____XXXX________,
   ____XXXX________,
   ________________}  /* char '7'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char '8'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXXXX__,
   __________XXXX__,
   ________XXXX____,
   __XXXXXXXX______,
   ________________}  /* char '9'  */

 ,{
   ________________,
   ______XXXX______,
   ______XXXX______,
   ________________,
   ________________,
   ______XXXX______,
   ______XXXX______,
   ________________}  /* char ':'  */

 ,{
   ________________,
   ______XXXX______,
   ______XXXX______,
   ________________,
   ________________,
   ______XXXX______,
   ______XXXX______,
   ____XXXX________}  /* char ';'  */

 ,{
   __________XXXX__,
   ________XXXX____,
   ______XXXX______,
   ____XXXX________,
   ______XXXX______,
   ________XXXX____,
   __________XXXX__,
   ________________}  /* char '<'  */

 ,{
   ________________,
   ________________,
   __XXXXXXXXXXXX__,
   ________________,
   ________________,
   __XXXXXXXXXXXX__,
   ________________,
   ________________}  /* char '='  */

 ,{
   __XXXX__________,
   ____XXXX________,
   ______XXXX______,
   ________XXXX____,
   ______XXXX______,
   ____XXXX________,
   __XXXX__________,
   ________________}  /* char '>'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   ________XXXX____,
   ______XXXX______,
   ______XXXX______,
   ________________,
   ______XXXX______,
   ________________}  /* char '?'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX__XXXXXXXX__,
   XXXX__XXXXXXXX__,
   XXXX__XXXXXXXX__,
   XXXX____________,
   __XXXXXXXX______,
   ________________}  /* char '@'  */

 ,{
   ____XXXXXX______,
   __XXXX__XXXX____,
   XXXX______XXXX__,
   XXXXXXXXXXXXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   ________________}  /* char 'A'  */

 ,{
   XXXXXXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   XXXXXXXXXXXX____,
   ________________}  /* char 'B'  */

 ,{
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   XXXX____________,
   XXXX____________,
   XXXX____________,
   __XXXX____XXXX__,
   ____XXXXXXXX____,
   ________________}  /* char 'C'  */

 ,{
   XXXXXXXXXX______,
   __XXXX__XXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX__XXXX____,
   XXXXXXXXXX______,
   ________________}  /* char 'D'  */

 ,{
   XXXXXXXXXXXXXX__,
   __XXXX______XX__,
   __XXXX__XX______,
   __XXXXXXXX______,
   __XXXX__XX______,
   __XXXX______XX__,
   XXXXXXXXXXXXXX__,
   ________________}  /* char 'E'  */

 ,{
   XXXXXXXXXXXXXX__,
   __XXXX______XX__,
   __XXXX__XX______,
   __XXXXXXXX______,
   __XXXX__XX______,
   __XXXX__________,
   XXXXXXXX________,
   ________________}  /* char 'F'  */

 ,{
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   XXXX____________,
   XXXX____________,
   XXXX____XXXXXX__,
   __XXXX____XXXX__,
   ____XXXXXX__XX__,
   ________________}  /* char 'G'  */

 ,{
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXXXXXXXXXXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   ________________}  /* char 'H'  */

 ,{
   ____XXXXXXXX____,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ________________}  /* char 'I'  */

 ,{
   ______XXXXXXXX__,
   ________XXXX____,
   ________XXXX____,
   ________XXXX____,
   XXXX____XXXX____,
   XXXX____XXXX____,
   __XXXXXXXX______,
   ________________}  /* char 'J'  */

 ,{
   XXXXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX__XXXX____,
   __XXXXXXXX______,
   __XXXX__XXXX____,
   __XXXX____XXXX__,
   XXXXXX____XXXX__,
   ________________}  /* char 'K'  */

 ,{
   XXXXXXXX________,
   __XXXX__________,
   __XXXX__________,
   __XXXX__________,
   __XXXX______XX__,
   __XXXX____XXXX__,
   XXXXXXXXXXXXXX__,
   ________________}  /* char 'L'  */

 ,{
   XXXX______XXXX__,
   XXXXXX__XXXXXX__,
   XXXXXXXXXXXXXX__,
   XXXXXXXXXXXXXX__,
   XXXX__XX__XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   ________________}  /* char 'M'  */

 ,{
   XXXX______XXXX__,
   XXXXXX____XXXX__,
   XXXXXXXX__XXXX__,
   XXXX__XXXXXXXX__,
   XXXX____XXXXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   ________________}  /* char 'N'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char 'O'  */

 ,{
   XXXXXXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXXXXXXXX____,
   __XXXX__________,
   __XXXX__________,
   XXXXXXXX________,
   ________________}  /* char 'P'  */

 ,{
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX____XXXXXX__,
   __XXXXXXXXXX____,
   ________XXXXXX__}  /* char 'Q'  */

 ,{
   XXXXXXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXXXXXXXX____,
   __XXXX__XXXX____,
   __XXXX____XXXX__,
   XXXXXX____XXXX__,
   ________________}  /* char 'R'  */

 ,{
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   ____XXXX________,
   ______XXXX______,
   ________XXXX____,
   __XXXX____XXXX__,
   ____XXXXXXXX____,
   ________________}  /* char 'S'  */

 ,{
   __XXXXXXXXXXXX__,
   __XXXXXXXXXXXX__,
   __XX__XXXX__XX__,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ________________}  /* char 'T'  */

 ,{
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char 'U'  */

 ,{
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXX__XXXX____,
   ____XXXXXX______,
   ________________}  /* char 'V'  */

 ,{
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX__XX__XXXX__,
   XXXX__XX__XXXX__,
   XXXXXXXXXXXXXX__,
   __XXXX__XXXX____,
   ________________}  /* char 'W'  */

 ,{
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXX__XXXX____,
   ____XXXXXX______,
   __XXXX__XXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   ________________}  /* char 'X'  */

 ,{
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   ____XXXXXXXX____,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ________________}  /* char 'Y'  */

 ,{
   XXXXXXXXXXXXXX__,
   XXXX______XXXX__,
   XX______XXXX____,
   ______XXXX______,
   ____XXXX____XX__,
   __XXXX____XXXX__,
   XXXXXXXXXXXXXX__,
   ________________}  /* char 'Z'  */

 ,{
   ____XXXXXXXX____,
   ____XXXX________,
   ____XXXX________,
   ____XXXX________,
   ____XXXX________,
   ____XXXX________,
   ____XXXXXXXX____,
   ________________}  /* char '['  */

 ,{
   XXXX____________,
   __XXXX__________,
   ____XXXX________,
   ______XXXX______,
   ________XXXX____,
   __________XXXX__,
   ____________XX__,
   ________________}  /* char '\'  */

 ,{
   ____XXXXXXXX____,
   ________XXXX____,
   ________XXXX____,
   ________XXXX____,
   ________XXXX____,
   ________XXXX____,
   ____XXXXXXXX____,
   ________________}  /* char ']'  */

 ,{
   ______XX________,
   ____XXXXXX______,
   __XXXX__XXXX____,
   XXXX______XXXX__,
   ________________,
   ________________,
   ________________,
   ________________}  /* char '^'  */

 ,{
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   XXXXXXXXXXXXXXXX}  /* char '_'  */

 ,{
   ____XXXX________,
   ______XXXX______,
   ________XXXX____,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}  /* char '`'  */

 ,{
   ________________,
   ________________,
   __XXXXXXXX______,
   ________XXXX____,
   __XXXXXXXXXX____,
   XXXX____XXXX____,
   __XXXXXX__XXXX__,
   ________________}  /* char 'a'  */

 ,{
   XXXXXX__________,
   __XXXX__________,
   __XXXXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   XXXX__XXXXXX____,
   ________________}  /* char 'b'  */

 ,{
   ________________,
   ________________,
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX____________,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char 'c'  */

 ,{
   ______XXXXXX____,
   ________XXXX____,
   __XXXXXXXXXX____,
   XXXX____XXXX____,
   XXXX____XXXX____,
   XXXX____XXXX____,
   __XXXXXX__XXXX__,
   ________________}  /* char 'd'  */

 ,{
   ________________,
   ________________,
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXXXXXXXXXXXX__,
   XXXX____________,
   __XXXXXXXXXX____,
   ________________}  /* char 'e'  */

 ,{
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX__________,
   XXXXXXXXXX______,
   __XXXX__________,
   __XXXX__________,
   XXXXXXXX________,
   ________________}  /* char 'f'  */

 ,{
   ________________,
   ________________,
   __XXXXXX__XXXX__,
   XXXX____XXXX____,
   XXXX____XXXX____,
   __XXXXXXXXXX____,
   ________XXXX____,
   XXXXXXXXXX______}  /* char 'g'  */

 ,{
   XXXXXX__________,
   __XXXX__________,
   __XXXX__XXXX____,
   __XXXXXX__XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   XXXXXX____XXXX__,
   ________________}  /* char 'h'  */

 ,{
   ______XXXX______,
   ________________,
   ____XXXXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ________________}  /* char 'i'  */

 ,{
   __________XXXX__,
   ________________,
   __________XXXX__,
   __________XXXX__,
   __________XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   ____XXXXXXXX____}  /* char 'j'  */

 ,{
   XXXXXX__________,
   __XXXX__________,
   __XXXX____XXXX__,
   __XXXX__XXXX____,
   __XXXXXXXX______,
   __XXXX__XXXX____,
   XXXXXX____XXXX__,
   ________________}  /* char 'k'  */

 ,{
   ____XXXXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ________________}  /* char 'l'  */

 ,{
   ________________,
   ________________,
   XXXXXX__XXXX____,
   XXXXXXXXXXXXXX__,
   XXXX__XX__XXXX__,
   XXXX__XX__XXXX__,
   XXXX__XX__XXXX__,
   ________________}  /* char 'm'  */

 ,{
   ________________,
   ________________,
   XXXX__XXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   ________________}  /* char 'n'  */

 ,{
   ________________,
   ________________,
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}  /* char 'o'  */

 ,{
   ________________,
   ________________,
   XXXX__XXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXXXXXXXX____,
   __XXXX__________,
   XXXXXXXX________}  /* char 'p'  */

 ,{
   ________________,
   ________________,
   __XXXXXX__XXXX__,
   XXXX____XXXX____,
   XXXX____XXXX____,
   __XXXXXXXXXX____,
   ________XXXX____,
   ______XXXXXXXX__}  /* char 'q'  */

 ,{
   ________________,
   ________________,
   XXXX__XXXXXX____,
   __XXXXXX__XXXX__,
   __XXXX__________,
   __XXXX__________,
   XXXXXXXX________,
   ________________}  /* char 'r'  */

 ,{
   ________________,
   ________________,
   __XXXXXXXXXXXX__,
   XXXX____________,
   __XXXXXXXXXX____,
   __________XXXX__,
   XXXXXXXXXXXX____,
   ________________}  /* char 's'  */

 ,{
   ____XXXX________,
   ____XXXX________,
   XXXXXXXXXXXX____,
   ____XXXX________,
   ____XXXX________,
   ____XXXX__XXXX__,
   ______XXXXXX____,
   ________________}  /* char 't'  */

 ,{
   ________________,
   ________________,
   XXXX____XXXX____,
   XXXX____XXXX____,
   XXXX____XXXX____,
   XXXX____XXXX____,
   __XXXXXX__XXXX__,
   ________________}  /* char 'u'  */

 ,{
   ________________,
   ________________,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXX__XXXX____,
   ____XXXXXX______,
   ________________}  /* char 'v'  */

 ,{
   ________________,
   ________________,
   XXXX______XXXX__,
   XXXX__XX__XXXX__,
   XXXX__XX__XXXX__,
   XXXXXXXXXXXXXX__,
   __XXXX__XXXX____,
   ________________}  /* char 'w'  */

 ,{
   ________________,
   ________________,
   XXXX______XXXX__,
   __XXXX__XXXX____,
   ____XXXXXX______,
   __XXXX__XXXX____,
   XXXX______XXXX__,
   ________________}  /* char 'x'  */

 ,{
   ________________,
   ________________,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXXXX__,
   __________XXXX__,
   XXXXXXXXXXXX____}  /* char 'y'  */

 ,{
   ________________,
   ________________,
   __XXXXXXXXXXXX__,
   __XX____XXXX____,
   ______XXXX______,
   ____XXXX____XX__,
   __XXXXXXXXXXXX__,
   ________________}  /* char 'z'  */

 ,{
   ________XXXXXX__,
   ______XXXX______,
   ______XXXX______,
   __XXXXXX________,
   ______XXXX______,
   ______XXXX______,
   ________XXXXXX__,
   ________________}  /* char '{'  */

 ,{
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ________________}  /* char '|'  */

 ,{
   __XXXXXX________,
   ______XXXX______,
   ______XXXX______,
   ________XXXXXX__,
   ______XXXX______,
   ______XXXX______,
   __XXXXXX________,
   ________________}  /* char '}'  */

 ,{
   __XXXXXX__XXXX__,
   XXXX__XXXXXX____,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}   /* char '~'  */



/*  additional characters for European character set  */

/* small accent ague, 95 */
 ,{
   ________XXXX____,
   ______XX________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}

 /* small accent grave, 96 */
 ,{
   __XXXX__________,
   ______XX________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


/* small accent circonflex, 97 */
 ,{
   __XXXXXXXXXX____,
   XX__________XX__,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* small umlaut, 98 */
 ,{
   XXXX______XXXX__,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* small tilde, 99  */
 ,{
   __XXXXXX____XX__,
   XX______XXXX____,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* i without dot, 100  */
 ,{
   ________________,
   ________________,
   ____XXXXXX______,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ________________}


 /* sharps eg �? 101  */
 ,{
   ____XXXXXX______,
   __XXXX__XXXX____,
   __XXXX__XXXX____,
   __XXXXXXXX______,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX__XXXX____,
   XXXX____________}


 /* capital A with ring, 102  */
 ,{
   ______XXXX______,
   ________________,
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   __XXXXXXXXXXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   ________________}


 /* small a with ring, 103  */
 ,{
   ____XXXX________,
   ________________,
   __XXXXXXXX______,
   ________XXXX____,
   __XXXXXXXXXX____,
   XXXX____XXXX____,
   __XXXXXX__XXXX__,
   ________________}



 /*  capital A diphtong, ligature, 104  */
 ,{
   ______XXXXXXXX__,
   __XXXXXXXX______,
   XX____XXXX______,
   XX____XXXXXXXX__,
   XXXXXXXXXX______,
   XX____XXXX______,
   XX____XXXXXXXX__,
   ________________}


 /*  Icelandic eth, 105  */
 ,{
   __XXXXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   XXXXXXXX__XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXXXXXXXX____,
   ________________}


 /*  Icelandic Thorn, 106  */
 ,{
   XXXXXXXX________,
   __XXXX__________,
   __XXXXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXXXXXXXX____,
   __XXXX__________,
   XXXXXXXX________}


 /*  small a diphtong, ligature, 107  */
 ,{
   ________________,
   ________________,
   XXXX____XXXX____,
   ____XXXX____XX__,
   __XXXXXXXXXXXX__,
   XX__XXXX________,
   XXXXXX__XXXX____,
   ________________}


 /*  small Icelanic Eth, 108 */
 ,{
   __XXXX__XX______,
   ____XXXX________,
   __XX__XXXX______,
   ________XXXX____,
   __XXXXXXXXXXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}


 /*  small Icelandic Thorn, 109 */
 ,{
   __XXXXXX________,
   ____XXXX________,
   ____XXXXXXXX____,
   ____XXXX__XXXX__,
   ____XXXXXXXX____,
   ____XXXX________,
   __XXXXXXXX______,
   ________________}


 /*  capital O with slash, 110 */
 ,{
   ____XXXXXX__XX__,
   __XXXX__XXXX____,
   XXXX____XXXXXX__,
   XXXX__XX__XXXX__,
   XXXXXX____XXXX__,
   __XXXX__XXXX____,
   XX__XXXXXX______,
   ________________}


 /*  small o with slash, 111 */
 ,{
   ________________,
   ____________XX__,
   __XXXXXXXXXX____,
   XXXX____XXXXXX__,
   XXXX__XX__XXXX__,
   XXXXXX____XXXX__,
   __XXXXXXXXXX____,
   XX______________}


 /*  Little capital A, 112 */
 ,{
   ________________,
   ________________,
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXXXXXXXXXXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   ________________}


 /*  Little capital E, 113 */
 ,{
   ________________,
   ________________,
   XXXXXXXXXXXXXX__,
   __XXXX__________,
   __XXXXXXXXXX____,
   __XXXX__________,
   XXXXXXXXXXXXXX__,
   ________________}


 /*  Little capital I, 114 */
 ,{
   ________________,
   ________________,
   ____XXXXXXXX____,
   ______XXXX______,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ________________}


 /*  Little capital O, 115 */
 ,{
   ________________,
   ________________,
   __XXXXXXXXXX____,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}


 /*  Little capital U, 116 */
 ,{
   ________________,
   ________________,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}


 /*  Little capital N, 117 */
 ,{
   ________________,
   ________________,
   XXXXXX____XXXX__,
   XXXXXXXX__XXXX__,
   XXXX__XXXXXXXX__,
   XXXX____XXXXXX__,
   XXXX______XXXX__,
   ________________}


 /*  inverted exclamation, 118 */

 ,{
   ______XXXX______,
   ________________,
   ______XXXX______,
   ______XXXX______,
   ____XXXXXXXX____,
   ____XXXXXXXX____,
   ______XXXX______,
   ________________}


 /*  inverted question mark, 119  */
 ,{
   ____XXXX________,
   ________________,
   ____XXXX________,
   ____XXXX________,
   __XXXX__________,
   XXXX______XXXX__,
   __XXXXXXXXXX____,
   ________________}


 /* capital accent ague, 120 */

 ,{
   ________XXXX____,
   ______XX________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* capital accent grave, 121 */
 ,{
   __XXXX__________,
   ______XX________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* capital accent circonflex, 122 */
 ,{
   ____XXXXXX______,
   __XX______XX____,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* capital umlaut, 123 */
 ,{
   XXXX______XXXX__,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* capital tilde, 124 */
 ,{
   __XXXXXX____XX__,
   XX______XXXX____,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /*  cedilla, 125  */
 ,{
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ______XX________}



/*  additional characters for complete ISO 8859-1 character set  */

/* Non breaking space, ISO-Code: 160, internal code: 126  */
 ,{
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* cent sign, ISO-Code: 162, internal code: 127  */
 ,{
   ________________,
   ______XX________,
   __XXXXXXXXXX____,
   XXXX__XX________,
   XXXX__XX________,
   XXXX__XX__XXXX__,
   __XXXXXXXXXX____,
   ______XX________}  /* char 'c'  */


 /* pound sterling, ISO-Code: 163, internal code: 128  */
 ,{
   ______XXXXXX____,
   ____XXXX________,
   ____XXXX________,
   __XXXXXXXX______,
   ____XXXX________,
   ____XXXX____XX__,
   __XX__XXXXXX____,
   ________________}


 /* general currency sign, ISO-Code: 164, internal code: 129  */
 ,{
   ________________,
   ________________,
   __XX______XX____,
   ____XXXXXX______,
   ____XX__XX______,
   ____XXXXXX______,
   __XX______XX____,
   ________________}


 /* yen sign, ISO-Code: 165, internal code: 130  */
 ,{
   __XX______XX____,
   ____XX__XX______,
   __XXXXXXXXXX____,
   ______XX________,
   __XXXXXXXXXX____,
   ______XX________,
   ______XX________,
   ________________}



 /* broken vertical bar, ISO-Code: 166, internal code: 131  */
 ,{
   ____XXXX________,
   ____XXXX________,
   ____XXXX________,
   ________________,
   ____XXXX________,
   ____XXXX________,
   ____XXXX________,
   ________________}


 /* section sign, ISO-Code: 167, internal code: 132  */
 ,{
   ____XXXXXX______,
   __XX____________,
   ____XXXX________,
   __XX____XX______,
   ____XXXX________,
   ________XX______,
   __XXXXXX________,
   ________________}


 /* umlaut (dieresis), ISO-Code: 168, internal code: 133  */
 ,{
   XXXX______XXXX__,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* copyright, ISO-Code: 169, internal code: 134  */
 ,{
   __XXXXXXXXXXXX__,
   __XX________XX__,
   __XX__XXXX__XX__,
   __XX__XXXXXXXX__,
   __XX__XXXX__XX__,
   __XX________XX__,
   __XXXXXXXXXXXX__,
   ________________}


 /* feminine ordinal, ISO-Code: 170, internal code: 135  */
 ,{
   ____XXXXXX______,
   __________XX____,
   ____XXXXXXXX____,
   __XX______XX____,
   ____XXXXXX______,
   ________________,
   ________________,
   ________________}


 /* left angle quote, ISO-Code: 172, internal code: 136  */
 ,{
   ________________,
   ____XXXX__XXXX__,
   __XXXX__XXXX____,
   XXXX__XXXX______,
   __XXXX__XXXX____,
   ____XXXX__XXXX__,
   ________________,
   ________________}


 /* not sign, ISO-Code: 173, internal code: 137  */
 ,{
   ________________,
   ________________,
   ________________,
   __XXXXXXXXXX____,
   __________XX____,
   __________XX____,
   ________________,
   ________________}


 /* soft hyphen, ISO-Code: 173, internal code: 138  */
 ,{
   ________________,
   ________________,
   ________________,
   __XXXXXXXXXX____,
   ________________,
   ________________,
   ________________,
   ________________}


 /* rgistered trademark, ISO-Code: 174, internal code: 139  */
 ,{
   __XXXXXXXXXX____,
   __XX______XX____,
   __XX__XX__XX____,
   __XX______XX____,
   __XX____XXXX____,
   __XX__XX__XX____,
   __XXXXXXXXXX____,
   ________________}


 /* macron accent, ISO-Code: 175, internal code: 140  */
 ,{
   __XXXXXXXXXX____,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* degree sign, ISO-Code: 176, internal code: 141  */
 ,{
   ____XXXX________,
   __XX____XX______,
   ____XXXX________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* plus or minus, ISO-Code: 177, internal code: 142  */
 ,{
   ______XX________,
   ______XX________,
   __XXXXXXXXXX____,
   ______XX________,
   ______XX________,
   ________________,
   __XXXXXXXXXX____,
   ________________}


 /* superscript two, ISO-Code: 178, internal code: 143  */
 ,{
   __XXXX__________,
   XX____XX________,
   ____XX__________,
   __XX____________,
   XXXXXXXX________,
   ________________,
   ________________,
   ________________}


 /* superscript three, ISO-Code: 179, internal code: 144  */
 ,{
   XXXXXX__________,
   ______XX________,
   __XXXX__________,
   ______XX________,
   XXXXXX__________,
   ________________,
   ________________,
   ________________}


 /* acute accent, ISO-Code: 180, internal code: 145  */
 ,{
   ________XXXXXX__,
   ______XXXX______,
   ____XX__________,
   ________________,
   ________________,
   ________________,
   ________________,
   ________________}


 /* micro sign, ISO-Code: 181, internal code: 146  */
 ,{
   ________________,
   ________________,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXXXXXXXX____,
   XXXX____________}


 /* paragraph sign, ISO-Code: 182, internal code: 147  */
 ,{
   __XXXXXXXXXXXX__,
   XXXX__XX__XX____,
   XXXX__XX__XX____,
   __XXXXXX__XX____,
   ______XX__XX____,
   ______XX__XX____,
   ____XXXXXXXXXX__,
   ________________}


 /* middle dot, ISO-Code: 183, internal code: 148  */
 ,{
   ________________,
   ________________,
   ________________,
   ______XXXX______,
   ______XXXX______,
   ________________,
   ________________,
   ________________}


 /* superscript one, ISO-Code: 185, internal code: 149  */
 ,{
   __XX____________,
   XXXX____________,
   __XX____________,
   __XX____________,
   XXXXXX__________,
   ________________,
   ________________,
   ________________}


 /* masculine ordinal, ISO-Code: 186, internal code: 150  */
 ,{
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   __XXXX____XXXX__,
   ____XXXXXXXX____,
   ________________,
   ________________,
   ________________}


 /* right angle quote, ISO-Code: 187, internal code: 151  */
 ,{
   ________________,
   XXXX__XXXX______,
   __XXXX__XXXX____,
   ____XXXX__XXXX__,
   __XXXX__XXXX____,
   XXXX__XXXX______,
   ________________,
   ________________}


 /* fraction one-fourth, ISO-Code: 188, internal code: 152  */
 ,{
   XX________XX____,
   XX______XX______,
   XX____XX________,
   XX__XX____XX____,
   __XX____XXXX____,
   XX____XX__XX____,
   ______XXXXXXXX__,
   __________XX____}


 /* fraction one-half, ISO-Code: 189, internal code: 153  */
 ,{
   XX________XX____,
   XX______XX______,
   XX____XX________,
   XX__XX__XXXX____,
   __XX__XX____XX__,
   XX________XX____,
   ________XX______,
   ______XXXXXXXX__}


 /* fraction three-fourth, ISO-Code: 190, internal code: 154  */
 ,{
   XXXX____________,
   ____XX__________,
   __XX____________,
   ____XX____XX____,
   XXXX____XXXX____,
   ______XX__XX____,
   ______XXXXXXXX__,
   __________XX____}


 /* multiply sign, ISO-Code: 215, internal code: 155  */
 ,{
   ________________,
   __XXXX____XXXX__,
   ____XXXXXXXX____,
   ______XXXX______,
   ____XXXXXXXX____,
   __XXXX____XXXX__,
   ________________,
   ________________}


 /* division sign, ISO-Code: 247, internal code: 156  */
 ,{
   ________________,
   ______XXXX______,
   ________________,
   __XXXXXXXXXXXX__,
   ________________,
   ______XXXX______,
   ________________,
   ________________}


 /* left arrow, ISO-Code: ---, internal code: 157  */
 ,{
   ______XX________,
   ____XXXX________,
   __XXXXXX________,
   XXXXXXXXXXXX____,
   __XXXXXX________,
   ____XXXX________,
   ______XX________,
   ________________}

 /* right arrow, ISO-Code: ---, internal code: 158  */
 ,{
   ____XX__________,
   ____XXXX________,
   ____XXXXXX______,
   XXXXXXXXXXXX____,
   ____XXXXXX______,
   ____XXXX________,
   ____XX__________,
   ________________}

 /* up arrow, ISO-Code: ---, internal code: 159 */
 ,{
   ________________,
   ____XX__________,
   __XXXXXX________,
   XXXXXXXXXX______,
   ____XX__________,
   ____XX__________,
   ________________,
   ________________}

 /* down arrow, ISO-Code: ---, internal code: 160  */
 ,{
   ________________,
   ____XX__________,
   ____XX__________,
   XXXXXXXXXX______,
   __XXXXXX________,
   ____XX__________,
   ________________,
   ________________}

 /* ENTER character, ISO-Code: ---, internal code: 162  */
 ,{
   ________________,
   ________XX______,
   ____XX__XX______,
   __XXXX__XX______,
   XXXXXXXXXX______,
   __XXXX__________,
   ____XX__________,
   ________________}

 /* ENTER character, ISO-Code: ---, internal code: 162  */
 ,{
   __________XXXX__,
   __________XXXX__,
   __________XXXX__,
   ________XXXX____,
   XX______XXXX____,
   XXXX__XXXX______,
   __XXXXXXXX______,
   ____XXXX________}


};


const GUI_FONT_TRANSLIST GUI_F8x8_TransList[] = {
/*
   The folowing are extensions to ISO 8859-1.
   Since ISO 8859-1 does not define any characters for the codes
   128 - 159, this area can be used by an application.
   The most commonly used symbols in embedded applications are
   therefor inserted here.
*/
  {CODE_ARROW_LEFT,-1},			/* 144, arrow left */
  {CODE_ARROW_RIGHT,-1},		/* 145, arrow right */
  {CODE_ARROW_UP,-1},			/* 146, arrow up */
  {CODE_ARROW_DOWN,-1},			/* 147, arrow down */
  {CODE_ENTER,-1},                      /* 148, enter symbol */
  {CODE_CHECKMARK,-1},			/* 149, checkmark symbol */
  {-1,-1},				/* 150, unused symbol */
  {-1,-1},				/* 151, unused symbol */
  {-1,-1},				/* 152, unused symbol */
  {-1,-1},				/* 153, unused symbol */
  {-1,-1},				/* 154, unused symbol */
  {-1,-1},				/* 155, unused symbol */
  {-1,-1},				/* 156, unused symbol */
  {-1,-1},				/* 157, unused symbol */
  {-1,-1},				/* 158, unused symbol */
  {-1,-1},				/* 159, unused symbol */
/* starting at character code 160 are the characters defined
   by ISO 8859-1
*/
  {CODE_NB_SPACE,-1},                   /* 160, non-breaking space */
  {CODE_INVEXCLAM,-1},                  /* 161, inverted exclamation sign */
  {CODE_CENT,-1},                       /* 162, cent sign */
  {CODE_POUND,-1},                      /* 163, pound sterling */
  {CODE_CURRENCY,-1},                   /* 164, general currency sign */
  {CODE_YEN,-1},                        /* 165, yen sign */
  {CODE_BROKEN_BAR,-1},                 /* 166, broken vertical bar */
  {CODE_SECTION,-1},                    /* 167, section sign */
  {CODE_DIERESIS,-1},                   /* 168, umlaut */
  {CODE_COPYRIGHT,-1},                  /* 169, copyright */
  {CODE_FEMININE,-1},                   /* 170,  */
  {CODE_LEFT_QUOTE,-1},                 /* 171,  */
  {CODE_NOT,-1},                        /* 172,  */
  {CODE_HYPHEN,-1},                     /* 173,  */
  {CODE_TRADEMARK,-1},                  /* 174,  */
  {CODE_MACRON,-1},                     /* 175,  */
  {CODE_DEGREE,-1},                     /* 176,  */
  {CODE_PLUS_MINUS,-1},                 /* 177,  */
  {CODE_SUPER_TWO,-1},                  /* 178,  */
  {CODE_SUPER_THREE,-1},                /* 179,  */
  {CODE_ACUTE,-1},                      /* 180,  */
  {CODE_MICRO,-1},                      /* 181,  */
  {CODE_PARAGRAPH,-1},                  /* 182,  */
  {CODE_MIDDLE_DOT,-1},                 /* 183,  */
  {CODE_CEDILLA,-1},                    /* 184,  */
  {CODE_SUPER_ONE,-1},                  /* 185,  */
  {CODE_MASCULINE,-1},                  /* 186,  */
  {CODE_RIGHT_QUOTE,-1},                /* 187,  */
  {CODE_ONE_FOURTH,-1},                 /* 188,  */
  {CODE_ONE_HALF,-1},                   /* 189,  */
  {CODE_THREE_FOURTH,-1},               /* 190,  */
  {CODE_INVQUEST,-1},                   /* 191,  */
  {CODE_LITTLE_A,CODE_CACCGRAV},        /* 192,  */
  {CODE_LITTLE_A,CODE_CACCAGUE},        /* 193,  */
  {CODE_LITTLE_A,CODE_CACCCIRC},        /* 194,  */
  {CODE_LITTLE_A,CODE_CTILDE},          /* 195,  */
  {CODE_LITTLE_A,CODE_CUMLAUT},         /* 196,  */
  {CODE_A_RING,-1},                     /* 197,  */
  {CODE_AE,-1},                         /* 198,  */
  {'C'-32,CODE_CEDILLA},                /* 199,  */
  {CODE_LITTLE_E,CODE_CACCGRAV},        /* 200,  */
  {CODE_LITTLE_E,CODE_CACCAGUE},        /* 201,  */
  {CODE_LITTLE_E,CODE_CACCCIRC},        /* 202,  */
  {CODE_LITTLE_E,CODE_CUMLAUT},         /* 203,  */
  {CODE_LITTLE_I,CODE_CACCGRAV},        /* 204,  */
  {CODE_LITTLE_I,CODE_CACCAGUE},        /* 205,  */
  {CODE_LITTLE_I,CODE_CACCCIRC},        /* 206,  */
  {CODE_LITTLE_I,CODE_CUMLAUT},         /* 207,  */
  {CODE_ETH,-1},                        /* 208,  */
  {CODE_LITTLE_N,CODE_CTILDE},          /* 209,  */
  {CODE_LITTLE_O,CODE_CACCGRAV},        /* 210,  */
  {CODE_LITTLE_O,CODE_CACCAGUE},        /* 211,  */
  {CODE_LITTLE_O,CODE_CACCCIRC},        /* 212,  */
  {CODE_LITTLE_O,CODE_CTILDE},          /* 213,  */
  {CODE_LITTLE_O,CODE_CUMLAUT},         /* 214,  */
  {CODE_MULTIPLY,-1},                   /* 215,  */
  {CODE_OSLASH,-1},                     /* 216,  */
  {CODE_LITTLE_U,CODE_CACCGRAV},        /* 217,  */
  {CODE_LITTLE_U,CODE_CACCAGUE},        /* 218,  */
  {CODE_LITTLE_U,CODE_CACCCIRC},        /* 219,  */
  {CODE_LITTLE_U,CODE_CUMLAUT},         /* 220,  */
  {'Y'-32,CODE_CACCAGUE},               /* 221,  */
  {CODE_THORN,-1},                      /* 222,  */
  {CODE_SHARPS,-1},                     /* 223,  */
  {'a'-32,CODE_SACCGRAV},               /* 224,  */
  {'a'-32,CODE_SACCAGUE},               /* 225,  */
  {'a'-32,CODE_SACCCIRC},               /* 226,  */
  {'a'-32,CODE_STILDE},                 /* 227,  */
  {'a'-32,CODE_SUMLAUT},                /* 228,  */
  {CODE_SA_RING,-1},                    /* 229,  */
  {CODE_SMALLAE,-1},                    /* 230,  */
  {'c'-32,CODE_CEDILLA},                /* 231,  */
  {'e'-32,CODE_SACCGRAV},               /* 232,  */
  {'e'-32,CODE_SACCAGUE},               /* 233,  */
  {'e'-32,CODE_SACCCIRC},               /* 234,  */
  {'e'-32,CODE_SUMLAUT},                /* 235,  */
  {CODE_I_NOPOINT,CODE_SACCGRAV},       /* 236,  */
  {CODE_I_NOPOINT,CODE_SACCAGUE},       /* 237,  */
  {CODE_I_NOPOINT,CODE_SACCCIRC},       /* 238,  */
  {CODE_I_NOPOINT,CODE_SUMLAUT},        /* 239,  */
  {CODE_SMALLETH,-1},                   /* 240, small eth, Icelandic */
  {'n'-32,CODE_STILDE},                 /* 241, small n, tilde */
  {'o'-32,CODE_SACCGRAV},               /* 242, small o, grave accent */
  {'o'-32,CODE_SACCAGUE},               /* 243, small o, acute accent */
  {'o'-32,CODE_SACCCIRC},               /* 244, small o, circumflex  */
  {'o'-32,CODE_STILDE},                 /* 245, small o, tilde  */
  {'o'-32,CODE_SUMLAUT},                /* 246, small o, umlaut  */
  {CODE_DIVISION,-1},                   /* 247, division sign */
  {CODE_SOSLASH,-1},                    /* 248, small o slash */
  {'u'-32,CODE_SACCAGUE},               /* 249,  */
  {'u'-32,CODE_SACCGRAV},               /* 250,  */
  {'u'-32,CODE_SACCCIRC},               /* 251,  */
  {'u'-32,CODE_SUMLAUT},                /* 252, small u, umlaut  */
  {'y'-32,CODE_SACCAGUE},               /* 253, small y, acute accent */
  {CODE_SMALLTHORN,-1},                 /* 254, small thorn, Icelandic  */
  {'y'-32,CODE_SUMLAUT},                /* 255, */
};

const GUI_FONT_TRANSINFO GUI_F8x8_TransInfo = {
  144                   /*  FirstChar  */
  ,255                  /*  LastChar   */
  ,GUI_F8x8_TransList   /*  const GUI_FONT_TRANSLIST* pList */
};

const GUI_FONT_MONO GUI_F8x8_Mono = {
 GUI_F8x8_acFont[0],
 GUI_F8x8_acFont[0],
 &GUI_F8x8_TransInfo,
 32,                    /* FirstChar */
 127,                   /* LastChar */
 8,                     /* XSize */
 8,                     /* XDist */
 1                      /* BytesPerLine */
};

const GUI_FONT GUI_Font8x8 = {
  GUI_FONTTYPE_MONO
  ,8
  ,8
  ,{&GUI_F8x8_Mono}
  , 7, 5, 7
};

const GUI_FONT GUI_Font8x9 = {
  GUI_FONTTYPE_MONO
  ,8
  ,9
  ,{&GUI_F8x8_Mono}
  , 7, 5, 7 };
