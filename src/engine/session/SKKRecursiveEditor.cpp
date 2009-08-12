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

SKKRecursiveEditor::SKKRecursiveEditor(SKKInputEnvironment* env)
    : env_(env), editor_(env), state_(SKKState(env, &editor_)) {
    // initialize widgets
    widgets_.push_back(env->InputSessionParameter()->Annotator());
    widgets_.push_back(env->InputSessionParameter()->CandidateWindow());
    widgets_.push_back(env->InputSessionParameter()->DynamicCompletor());
    widgets_.push_back(env->InputModeSelector());
}

SKKRecursiveEditor::~SKKRecursiveEditor() {
    forEachWidget(&SKKWidget::Hide);
}

void SKKRecursiveEditor::Dispatch(const SKKEvent& event) {
    state_.Dispatch(SKKStateMachine::Event(event.id, event));
}

void SKKRecursiveEditor::UpdateInputContext() {
    editor_.UpdateInputContext();
}

void SKKRecursiveEditor::Commit(const std::string& word) {
    if(!word.empty()) {
        Dispatch(SKKEvent(SKK_ENTER, 0));

        editor_.Register(word);
    } else {
        Dispatch(SKKEvent(SKK_CANCEL, 0));
    }
}

void SKKRecursiveEditor::Activate() {
    forEachWidget(&SKKWidget::Activate);
}

void SKKRecursiveEditor::Deactivate() {
    forEachWidget(&SKKWidget::Deactivate);
}

// ----------------------------------------------------------------------

void SKKRecursiveEditor::forEachWidget(WidgetMethod method) {
    std::for_each(widgets_.begin(), widgets_.end(), std::mem_fun(method));
}
