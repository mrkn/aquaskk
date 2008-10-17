/* -*- mode: C++; coding: utf-8 -*-

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

#include "SKKComposingEditor.h"
#include "SKKContextBuffer.h"
#include "SKKBackEnd.h"
#include "jconv.h"
#include <iostream>

SKKComposingEditor::SKKComposingEditor() : enableDynamicCompletion_(false) {
    setModified();
}

void SKKComposingEditor::Input(const std::string& fixed, const std::string&) {
    setModified();

    composing_.Insert(fixed);
    updateCompletion();
}

void SKKComposingEditor::Input(SKKBaseEditor::Event event) {
    setModified();

    switch(event) {
    case BackSpace:
        if(composing_.IsEmpty()) {
            modified_ = false;
        }
        composing_.BackSpace();
        updateCompletion();
        break;

    case Delete:
        composing_.Delete();
        updateCompletion();
        break;

    case CursorLeft:
        composing_.CursorLeft();
        break;
        
    case CursorRight:
        composing_.CursorRight();
        break;
        
    case CursorUp:
        composing_.CursorUp();
        break;

    case CursorDown:
        composing_.CursorDown();
        break;

    default:
        return;
    }
}

void SKKComposingEditor::Clear() {
    setModified();

    composing_.Clear();
    completion_.clear();
}

void SKKComposingEditor::Output(SKKContextBuffer& buffer) const {
    if(completion_.empty()) {
        buffer.Compose("▽" + composing_.String(), composing_.CursorPosition());
    } else {
        buffer.Compose("▽" + composing_.String(), completion_, composing_.CursorPosition());
    }

    buffer.SetEntry(SKKEntry(composing_.LeftString()));
}

void SKKComposingEditor::Commit(std::string& queue) {
    queue = composing_.String();

    Clear();
}

void SKKComposingEditor::Flush() {
    setModified();
}

bool SKKComposingEditor::IsModified() const {
    return modified_;
}

const std::string SKKComposingEditor::QueryString() const {
    return composing_.LeftString();
}

void SKKComposingEditor::SetEntry(const std::string& entry) {
    Clear();

    composing_.Insert(entry);
}

void SKKComposingEditor::EnableDynamicCompletion(bool flag) {
    enableDynamicCompletion_ = flag;
}

// ------------------------------------------------------------

void SKKComposingEditor::setModified() {
    modified_ = true;
}

void SKKComposingEditor::updateCompletion() {
    if(!enableDynamicCompletion_) return;

    std::vector<std::string> result;
    std::string key = composing_.String();

    // 最初の候補を補完する
    if(!key.empty() && SKKBackEnd::theInstance().Complete(key, result)) {
        completion_ = result[0];
    } else {
        completion_.clear();
    }
}
