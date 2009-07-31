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

#include "SKKRecursiveEditor.h"
#include "SKKInputSessionParameter.h"
#include "SKKAnnotator.h"
#include "SKKCandidateWindow.h"
#include "SKKDynamicCompletor.h"
#include "SKKBackEnd.h"

SKKRecursiveEditor::SKKRecursiveEditor(SKKRegistrationObserver* registrationObserver,
                                       SKKInputSessionParameter* param,
                                       SKKInputModeSelector* inputModeSelector,
                                       SKKBaseEditor* bottom)
    : bottom_(bottom)
    , param_(param)
    , inputModeSelector_(*inputModeSelector)
    , editor_(registrationObserver,
              &inputModeSelector_,
              bottom_.get(),
              param_)
    , state_(SKKState(param_->Messenger(),
                      param_->CandidateWindow(),
                      param_->StateConfiguration(),
                      &editor_)) {
    state_.Start();

    widgets_.push_back(param_->Annotator());
    widgets_.push_back(param_->CandidateWindow());
    widgets_.push_back(param_->DynamicCompletor());
    widgets_.push_back(&inputModeSelector_);
}

SKKRecursiveEditor::~SKKRecursiveEditor() {
    std::for_each(widgets_.begin(), widgets_.end(), std::mem_fun(&SKKWidget::Hide));
}

void SKKRecursiveEditor::Dispatch(const SKKEvent& event) {
    state_.Dispatch(SKKStateMachine::Event(event.id, event));
}

bool SKKRecursiveEditor::Output() {
    bool result = editor_.IsModified();

    editor_.Output();

    return result;
}

void SKKRecursiveEditor::Clear() {
    editor_.Reset();
}

bool SKKRecursiveEditor::IsComposing() const {
    return editor_.IsComposing();
}

void SKKRecursiveEditor::Commit(const std::string& word) {
    if(!word.empty()) {
        const SKKEntry entry = editor_.Entry();

        SKKBackEnd::theInstance().Register(entry, SKKCandidate(word, false));

        if(entry.IsOkuriAri()) {
            editor_.Insert(word + entry.OkuriString());
        } else {
            editor_.Insert(word);
        }

        Dispatch(SKKEvent(SKK_ENTER, 0));
    } else {
        Dispatch(SKKEvent(SKK_CANCEL, 0));
    }
}

void SKKRecursiveEditor::Activate() {
    std::for_each(widgets_.begin(), widgets_.end(), std::mem_fun(&SKKWidget::Activate));
}

void SKKRecursiveEditor::Deactivate() {
    std::for_each(widgets_.begin(), widgets_.end(), std::mem_fun(&SKKWidget::Deactivate));
}

const SKKEntry SKKRecursiveEditor::Entry() const {
    return editor_.Entry();
}

const std::string SKKRecursiveEditor::Word() const {
    return editor_.Word();
}
