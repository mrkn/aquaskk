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
#include "SKKCandidateWindow.h"
#include "SKKBackEnd.h"

SKKRecursiveEditor::SKKRecursiveEditor(SKKRegistrationObserver* registrationObserver,
                                       SKKInputSessionParameter* param,
                                       SKKInputModeSelector* inputModeSelector,
                                       SKKBaseEditor* bottom)
    : bottom_(bottom)
    , param_(param)
    , inputModeSelector_(*inputModeSelector)
    , editor_(param_->InputEngineOption(),
              registrationObserver,
              param_->FrontEnd(),
              &inputModeSelector_,
              param_->Clipboard(),
              bottom_.get())
    , state_(SKKState(param_->CandidateWindow(),
                      param_->StateConfiguration(),
                      &editor_)) {
    state_.Start();

    // *** FIXME ***
    // 初期遷移により入力モードが初期化され、editor_ の内部変更フラグが
    // 立つので、これをクリアする
    //
    // 理由：次のイベントが必ず「処理済み」になってしまうのを防ぐため
    //
    // ※本来はこのような知識は不要(設計にゆがみがある証拠)
    editor_.Reset(true);
}

void SKKRecursiveEditor::Dispatch(const SKKEvent& event) {
    state_.Dispatch(SKKStateMachine::Event(event.id, event));
}

bool SKKRecursiveEditor::Output() {
    bool result = editor_.IsModified();

    editor_.Output();

    return result;
}

void SKKRecursiveEditor::Commit(const std::string& word) {
    if(!word.empty()) {
        SKKBackEnd::theInstance().Register(editor_.Entry(), SKKCandidate(word, false));

        editor_.Insert(word);

        Dispatch(SKKEvent(SKK_ENTER, 0));
    } else {
        Dispatch(SKKEvent(SKK_CANCEL, 0));
    }
}

void SKKRecursiveEditor::Activate() {
    inputModeSelector_.Activate();
    param_->CandidateWindow()->Activate();
}

void SKKRecursiveEditor::Deactivate() {
    inputModeSelector_.Deactivate();
    param_->CandidateWindow()->Deactivate();
}

const SKKEntry SKKRecursiveEditor::Entry() const {
    return editor_.Entry();
}

const std::string SKKRecursiveEditor::Word() const {
    return editor_.Word();
}
