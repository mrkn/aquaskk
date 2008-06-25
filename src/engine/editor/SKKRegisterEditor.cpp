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

#include "SKKRegisterEditor.h"
#include "SKKContextBuffer.h"

SKKRegisterEditor::SKKRegisterEditor(const SKKEntry& entry) {
    prompt_ = "[登録：";

    if(entry.IsOkuriAri()) {
        prompt_ += entry.OkuriPrompt();
    } else {
        prompt_ += entry.EntryString();
    }

    prompt_ += "] ";
}

void SKKRegisterEditor::Input(const std::string& direct) {
    word_.Insert(direct);
}

void SKKRegisterEditor::Input(const std::string& fixed, const std::string& input) {
    word_.Insert(fixed);
    input_ = input;
}

void SKKRegisterEditor::Input(SKKBaseEditor::Event event) {
    switch(event) {
    case BackSpace:
        word_.BackSpace();
        break;

    case Delete:
        word_.Delete();
        break;
        
    case CursorLeft:
        word_.CursorLeft();
        break;

    case CursorRight:
        word_.CursorRight();
        break;

    case CursorUp:
        word_.CursorUp();
        break;

    case CursorDown:
        word_.CursorDown();
        break;

    default:
        return;
    }
}

void SKKRegisterEditor::Clear() {
    input_.clear();
}

void SKKRegisterEditor::Output(SKKContextBuffer& buffer, bool active) const {
    SKKTextBuffer tmp(word_);

    if(active) {
        tmp.Insert(input_);
    }

    buffer.Compose(prompt_ + tmp.String(), tmp.CursorPosition());
}

void SKKRegisterEditor::Commit(std::string& queue) {
    word_.Insert(queue);
    input_.clear();
    queue = word_.String();
}
