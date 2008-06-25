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
#include "SKKInputEngine.h"
#include "SKKInputModeWindow.h"
#include "SKKCandidateWindow.h"
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

SKKInputSession::SKKInputSession(SKKInputSessionParameter* param) : param_(param) {
    pushEngine();

    preventReentrantCall_ = false;
}

SKKInputSession::~SKKInputSession() {
    while(!stack_.empty()) {
        popEngine();
    }
}

bool SKKInputSession::HandleEvent(const SKKEvent& event) {
    if(preventReentrantCall_) return false;

    ScopedFlag on(preventReentrantCall_);

    event_ = SKKRegistrationObserver::None;
    bool finished = false;

    // イベント処理
    top()->Dispatch(event);

    // 再帰的辞書登録
    switch(event_) {
    case SKKRegistrationObserver::Begin:
	pushEngine();
	break;

    case SKKRegistrationObserver::Finish:
        if(stack_.size() == 1) {
            finished = true;
        }
        finish(true);
	break;

    case SKKRegistrationObserver::Abort:
        finish(false);
	break;
    }

    // 表示更新
    bool result = top()->Emit();

    if(finished) {
        return false;
    }

    return result;
}

void SKKInputSession::Activate() {
    // 候補ウィンドウの表示
    param_->CandidateWindow()->Activate();
    param_->InputModeWindow()->Activate();
}

void SKKInputSession::Deactivate() {
    // 候補ウィンドウを隠す
    param_->CandidateWindow()->Deactivate();
    param_->InputModeWindow()->Deactivate();
}

SKKInputEngine* SKKInputSession::top() {
    return stack_.back();
}

void SKKInputSession::pushEngine() {
    SKKBaseEditor* editor;

    if(stack_.empty()) {
        editor = new SKKPrimaryEditor();
    } else {
        editor = new SKKRegisterEditor(top()->Entry());
    }

    stack_.push_back(new SKKInputEngine(this, param_, editor));
}

void SKKInputSession::popEngine() {
    delete top();
    stack_.pop_back();
}

void SKKInputSession::finish(bool commit) {
    if(stack_.size() == 1) return;

    std::string word(top()->Word());

    popEngine();

    if(commit && !word.empty()) {
        top()->Commit(word);
    } else {
        top()->Cancel();
    }
}

void SKKInputSession::SKKRegistrationUpdate(SKKRegistrationObserver::Event event) {
    event_ = event;
}
