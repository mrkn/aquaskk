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
#include "SKKRecursiveEditor.h"
#include "SKKFrontEnd.h"
#include "SKKDynamicCompletor.h"
#include "SKKAnnotator.h"
#include "SKKPrimaryEditor.h"
#include "SKKBackEnd.h"

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
    : param_(param), context_(param->FrontEnd()), inEvent_(false) {
    stack_.push_back(createEditor(new SKKPrimaryEditor(&context_)));
}

SKKInputSession::~SKKInputSession() {
    while(!stack_.empty()) {
        popEditor();
    }

    while(!listeners_.empty()) {
        delete listeners_.back();
        listeners_.pop_back();
    }
}

void SKKInputSession::AddInputModeListener(SKKInputModeListener* listener) {
    listeners_.push_back(listener);
}

bool SKKInputSession::HandleEvent(const SKKEvent& event) {
    if(inEvent_) return false;

    scoped_flag on(inEvent_);

    beginEvent();

    top()->Dispatch(event);

    endEvent();

    output();

    return result(event);
}

void SKKInputSession::Clear() {
    if(inEvent_) return;

    scoped_flag on(inEvent_);

    while(stack_.size()) {
        popEditor();
    }

    stack_.push_back(createEditor(new SKKPrimaryEditor(&context_)));

    output();
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
    return new SKKRecursiveEditor(
        new SKKInputEnvironment(this, &context_, param_.get(), &listeners_, bottom));
}

void SKKInputSession::popEditor() {
    delete top();
    stack_.pop_back();
}

void SKKInputSession::beginEvent() {
    temp_ = stack_;

    context_.output.Clear();
    context_.event_handled = true;
    context_.needs_go_back = false;
}

void SKKInputSession::endEvent() {
    if(stack_.size() == temp_.size()) return;

    if(stack_.size() < temp_.size()) {
        stack_.push_back(temp_.back());
    } else {
        popEditor();
    }
}

void SKKInputSession::output() {
    context_.output.Clear();

    top()->SyncInputContext();

    context_.output.Output();

    SKKInputEngineOption* option = param_->InputEngineOption();
    SKKDynamicCompletor* completer = param_->DynamicCompletor();
    SKKAnnotator* annotator = param_->Annotator();

    if(context_.dynamic_completion && option->EnableDynamicCompletion()) {
        unsigned range = option->DynamicCompletionRange();
        std::string completion = complete(range);

        completer->Update(completion, context_.output.EntryCursor());
        completion.empty() ? completer->Hide() : completer->Show();
    } else {
        completer->Hide();
    }

    if(context_.annotator && option->EnableAnnotation()) {
        SKKCandidate candidate = context_.candidate;

        if(candidate.IsEmpty()) {
            annotator->Hide();
        } else {
            annotator->Update(candidate, context_.output.LeftString());
            annotator->Show();
        }
    } else {
        annotator->Hide();
    }
}

bool SKKInputSession::result(const SKKEvent& event) {
    if(1 < stack_.size()) {
        context_.event_handled = true;
    } else {
        // 単語登録中ではなく、未確定状態でもない
        if(!context_.output.IsComposing()) {
            switch(event.option) {
            case AlwaysHandled:     // 常に処理済み
                return true;

            case PseudoHandled:     // 未処理
                return false;
            }
        }
    }

    return context_.event_handled;
}

std::string SKKInputSession::complete(unsigned range) {
    SKKEntry entry = context_.entry;
    std::string completion;

    if(entry.IsEmpty() || entry.IsOkuriAri()) {
        return completion;
    }

    std::vector<std::string> result;
    std::string key = entry.EntryString();

    // 候補を補完する
    if(!key.empty() && SKKBackEnd::theInstance().Complete(key, result, range)) {
        unsigned limit = std::min((unsigned)result.size(), range);
        for(unsigned i = 0; i < limit; ++ i) { 
            completion += result[i];
            completion += "\n";
        }
        completion.erase(completion.size() - 1);
    }

    return completion;
}

// ----------------------------------------------------------------------
// SKKRegistrationObserver インタフェース
// ----------------------------------------------------------------------

void SKKInputSession::SKKRegistrationBegin(SKKBaseEditor* bottom) {
    temp_.push_back(createEditor(bottom));
}

void SKKInputSession::SKKRegistrationFinish(const std::string& word) {
    if(temp_.size() == 1) {
        context_.event_handled = false;
    } else {
        temp_.pop_back();
        temp_.back()->Commit(word);
    }
}

void SKKInputSession::SKKRegistrationCancel() {
    SKKRegistrationFinish("");
}
