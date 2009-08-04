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

#include "SKKEntryRemoveEditor.h"
#include "SKKContextBuffer.h"
#include "SKKBackEnd.h"

void SKKEntryRemoveEditor::Initialize(const SKKEntry& entry, const SKKCandidate& candidate) {
    prompt_ = entry.EntryString() + " /" + candidate.ToString() + "/ を削除しますか？(yes/no) ";

    entry_ = entry;
    candidate_ = candidate;

    Clear();
}

void SKKEntryRemoveEditor::Input(const std::string& fixed, const std::string&, char) {
    input_ += fixed;
}

void SKKEntryRemoveEditor::Input(Event event) {
    if(event == SKKBaseEditor::BackSpace && !input_.empty()) {
        input_.erase(input_.end() - 1);
    }
}

void SKKEntryRemoveEditor::Clear() {
    input_.clear();
}

void SKKEntryRemoveEditor::Output(SKKContextBuffer& buffer) const {
    SKKContextBuffer tmp;
    
    tmp.Compose(prompt_ + input_);
    tmp.SetEntry(SKKEntry(input_));

    buffer = tmp;
}

void SKKEntryRemoveEditor::Commit(std::string& queue) {
    SKKBackEnd::theInstance().Remove(entry_, candidate_);

    queue.clear();

    Clear();
}
