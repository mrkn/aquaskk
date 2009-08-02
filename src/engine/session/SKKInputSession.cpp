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

#include "SKKInputSession.h"
#include "SKKInputSessionParameter.h"
#include "SKKInputModeSelector.h"
#include "SKKRecursiveEditor.h"
#include "SKKPrimaryEditor.h"

namespace {
    class scoped_flag {
        bool& flag_;

    public:
        scoped_flag(bool& flag) : flag_(flag) {
            flag_ = true;
        }

        ~scoped_flag() {
            flag_ = false;
        }
    };
}

SKKInputSession::SKKInputSession(SKKInputSessionParameter* param)
    : param_(param), inEvent_(false) {
    stack_.push_back(createEditor(new SKKPrimaryEditor()));
}

SKKInputSession::~SKKInputSession() {
    while(!stack_.empty()) {
        popEditor();
    }

    selector_.DeleteAllListener();
}

void SKKInputSession::AddInputModeListener(SKKInputModeListener* listener) {
    selector_.AddListener(listener);
}

bool SKKInputSession::HandleEvent(const SKKEvent& event) {
    if(inEvent_) return false;

    scoped_flag on(inEvent_);

    beginEvent();

    top()->Dispatch(event);

    endEvent();

    bool handled = top()->Output();

    // 単語登録中ではなく、未確定状態でもない
    if(stack_.size() == 1 && !top()->IsComposing()) {
        // 処理オプションを調べる
        switch(event.option) {
        case AlwaysHandled:     // 常に処理済み
            return true;

        case PseudoHandled:     // 未処理
            return false;
        }
    }

    return handled;
}

void SKKInputSession::Clear() {
    if(inEvent_) return;

    scoped_flag on(inEvent_);

    while(stack_.size() != 1) {
        popEditor();
    }

    top()->Clear();
}

void SKKInputSession::Activate() {
    top()->Activate();
}

void SKKInputSession::Deactivate() {
    top()->Deactivate();
}

// ----------------------------------------------------------------------

SKKRecursiveEditor* SKKInputSession::top() {
    return stack_.back();
}

SKKRecursiveEditor* SKKInputSession::createEditor(SKKBaseEditor* bottom) {
    return new SKKRecursiveEditor(this, param_.get(), bottom, &selector_);
}

void SKKInputSession::popEditor() {
    delete top();
    stack_.pop_back();
}

void SKKInputSession::beginEvent() {
    temp_ = stack_;
}

void SKKInputSession::endEvent() {
    if(stack_.size() == temp_.size()) return;

    if(stack_.size() < temp_.size()) {
        stack_.push_back(temp_.back());
    } else {
        popEditor();
    }
}

// ----------------------------------------------------------------------
// SKKRegistrationObserver インタフェース
// ----------------------------------------------------------------------

void SKKInputSession::SKKRegistrationBegin(SKKBaseEditor* bottom) {
    temp_.push_back(createEditor(bottom));
}

void SKKInputSession::SKKRegistrationFinish(const std::string& word) {
    if(temp_.size() == 1) {
        top()->Output();
    } else {
        temp_.pop_back();
        temp_.back()->Commit(word);
    }
}

void SKKInputSession::SKKRegistrationCancel() {
    SKKRegistrationFinish("");
}
