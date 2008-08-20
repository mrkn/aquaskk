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

#include "SKKOkuriEditor.h"
#include "SKKContextBuffer.h"
#include "utf8util.h"
#include <iostream>
#include <cctype>
#include <exception>

SKKOkuriEditor::SKKOkuriEditor() : modified_(true) {}

void SKKOkuriEditor::Initialize(char prefix) {
    modified_ = true;
    first_ = true;

    entry_.clear();
    prefix_.clear();
    okuri_.clear();

    prefix_ += std::tolower(prefix);
}

bool SKKOkuriEditor::IsOkuriComplete() const {
    return !okuri_.empty();
}

void SKKOkuriEditor::Input(const std::string& fixed, const std::string& input) {
    if(first_) {
        first_ = false;

        // KesSi 対応
        if(!fixed.empty() && !input.empty()) {
            entry_ = fixed;
            return;
        }
    }

    okuri_ += fixed;
}

void SKKOkuriEditor::Input(SKKBaseEditor::Event event) {
    if(event == BackSpace) {
        if(!okuri_.empty()) {
            utf8::pop(okuri_);
        } else {
            modified_ = false;
        }
    }
}

void SKKOkuriEditor::Clear() {
    // 何もしない
}

void SKKOkuriEditor::Output(SKKContextBuffer& buffer) const {
    buffer.Compose(entry_ + "*" + okuri_);

    std::string tmp(buffer.Entry().EntryString());

    SKKEntry entry(tmp + entry_);
    entry.SetOkuri(prefix_, okuri_);

    buffer.SetEntry(entry);
}

void SKKOkuriEditor::Commit(std::string&) {
    entry_.clear();
    prefix_.clear();
    okuri_.clear();
}

void SKKOkuriEditor::Flush() {
    modified_ = true;
}

bool SKKOkuriEditor::IsModified() const {
    return modified_;
}
