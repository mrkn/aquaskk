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

#include "SKKInputModeSelector.h"
#include "SKKInputModeListener.h"
#include <algorithm>
#include <functional>

SKKInputModeSelector::SKKInputModeSelector()
    : SKKWidget(true), mode_(InvalidInputMode) {
    Select(HirakanaInputMode);
}

void SKKInputModeSelector::AddListener(SKKInputModeListener* listener) {
    listeners_.push_back(listener);
}

void SKKInputModeSelector::DeleteAllListener() {
    while(!listeners_.empty()) {
        delete listeners_.back();
        listeners_.pop_back();
    }
}

void SKKInputModeSelector::Select(SKKInputMode mode) {
    needsUpdate_ = mode_ != mode;
    mode_ = mode;

    std::for_each(listeners_.begin(), listeners_.end(),
                  std::bind2nd(std::mem_fun(&SKKInputModeListener::SelectInputMode), mode_));
}

void SKKInputModeSelector::Notify() {
    if(needsUpdate_) {
        needsUpdate_ = false;
        SKKWidgetShow();
    }
}

SKKInputModeSelector::operator SKKInputMode() const {
    return mode_;
}

// ------------------------------------------------------------

void SKKInputModeSelector::SKKWidgetShow() {
    std::for_each(listeners_.begin(), listeners_.end(), std::mem_fun(&SKKWidget::Show));
}

void SKKInputModeSelector::SKKWidgetHide() {
    std::for_each(listeners_.begin(), listeners_.end(), std::mem_fun(&SKKWidget::Hide));
}
