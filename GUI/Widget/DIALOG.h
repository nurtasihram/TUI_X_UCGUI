#pragma once

#include "WM.h"
#include "DIALOG_Intern.h"

import TUX.Widget.Button;
import TUX.Widget.CheckBox;
import TUX.Widget.DropDown;
import TUX.Widget.Edit;
import TUX.Widget.Frame;
import TUX.Widget.ListBox;
import TUX.Widget.MultEdit;
import TUX.Widget.Radio;
import TUX.Widget.Slider;
import TUX.Widget.Text;

WObj * WINDOW_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WObj * hWinParent, int x0, int y0, WM_CALLBACK *cb);
