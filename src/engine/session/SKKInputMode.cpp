/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2008 Tomotaka SUWA <t.suwa@mac.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "SKKInputMode.h"
#include "SKKInputModeWindow.h"
#include <algorithm>

SKKInputModeSelector::SKKInputModeSelector(SKKInputModeWindow* window) : window_(window) {
    Select(HirakanaInputMode);
}

void SKKInputModeSelector::Select(SKKInputMode mode) {
    mode_ = mode;
}

void SKKInputModeSelector::Notify() {
    window_->SelectInputMode(mode_);
}

void SKKInputModeSelector::Activate() {
    window_->Show();
}

SKKInputModeSelector::operator SKKInputMode() const {
    return mode_;
}
