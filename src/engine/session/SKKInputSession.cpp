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
#include "SKKRecursiveEditor.h"
#include "SKKPrimaryEditor.h"
#include "SKKRegisterEditor.h"

namespace {
    class ScopedFlag {
        bool& flag_;

    public:
        ScopedFlag(bool& flag) : flag_(flag) {
            flag_ = true;
        }

        ~ScopedFlag() {
            flag_ = false;
        }
    };
}

SKKInputSession::SKKInputSession(SKKInputSessionParameter* param, SKKInputModeSelector* master)
    : param_(param)
    , master_(master) {
    pushEditor();

    event_ = SKKRegistrationObserver::None;
    preventReentrantCall_ = false;
}

SKKInputSession::~SKKInputSession() {
    while(!stack_.empty()) {
        popEditor();
    }
}

bool SKKInputSession::HandleEvent(const SKKEvent& event) {
    if(preventReentrantCall_) return false;

    ScopedFlag on(preventReentrantCall_);

    top()->Dispatch(event);

    handleRegistrationEvent();

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
    if(preventReentrantCall_) return;

    ScopedFlag on(preventReentrantCall_);

    while(!stack_.empty()) {
        popEditor();
    }

    pushEditor();
}

void SKKInputSession::Activate() {
    top()->Activate();
}

void SKKInputSession::Deactivate() {
    top()->Deactivate();
}

SKKRecursiveEditor* SKKInputSession::top() {
    return stack_.back();
}

void SKKInputSession::pushEditor() {
    SKKBaseEditor* editor;

    if(stack_.empty()) {
        editor = new SKKPrimaryEditor();
    } else {
        editor = new SKKRegisterEditor(top()->Entry());
    }

    stack_.push_back(new SKKRecursiveEditor(this, param_, master_.get(), editor));
}

void SKKInputSession::popEditor() {
    delete top();

    stack_.pop_back();
}

void SKKInputSession::handleRegistrationEvent() {
    switch(event_) {
    case SKKRegistrationObserver::Begin:
	pushEditor();
	break;

    case SKKRegistrationObserver::Finish:
        if(stack_.size() == 1) {
            top()->Output();
        } else {
            commit(top()->Word());
        }
	break;

    case SKKRegistrationObserver::Abort:
        commit();
	break;
    }

    event_ = SKKRegistrationObserver::None;
}

void SKKInputSession::commit(const std::string& word) {
    if(stack_.size() == 1) return;

    popEditor();

    top()->Commit(word);
}

void SKKInputSession::SKKRegistrationUpdate(SKKRegistrationObserver::Event event) {
    event_ = event;
}
