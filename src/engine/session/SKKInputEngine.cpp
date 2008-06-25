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

#include "SKKInputEngine.h"
#include "SKKInputSessionParameter.h"
#include "SKKBackEnd.h"

SKKInputEngine::SKKInputEngine(SKKRegistrationObserver* registrationObserver,
                               SKKInputSessionParameter* param,
                               SKKBaseEditor* bottom)
    : param_(param), bottom_(bottom), 
      editor_(registrationObserver, param_->InputModeWindow(), param_->Clipboard(), bottom_.get()),
      state_(SKKState(param_->StateConfiguration(), &editor_, param_->CandidateWindow())) {
    state_.Start();
}

void SKKInputEngine::Dispatch(const SKKEvent& event) {
    state_.Dispatch(SKKStateMachine::Event(event.id, event));
}

bool SKKInputEngine::Emit() {
    bool result = editor_.IsModified();

    editor_.Output(param_->FrontEnd());

    return result;
}

void SKKInputEngine::Commit(const std::string& word) {
    SKKBackEnd::theInstance().Register(editor_.Entry(), word);

    editor_.Paste(word);

    Dispatch(SKKEvent(SKK_ENTER, 0));
}

void SKKInputEngine::Cancel() {
    Dispatch(SKKEvent(SKK_CANCEL, 0));
}

const SKKEntry SKKInputEngine::Entry() const {
    return editor_.Entry();
}

const std::string SKKInputEngine::Word() const {
    return editor_.Word();
}
