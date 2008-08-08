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

#include "SKKCandidateEditor.h"
#include "SKKContextBuffer.h"
#include "SKKBackEnd.h"

void SKKCandidateEditor::Initialize(const SKKEntry& entry) {
    entry_ = entry;
}

void SKKCandidateEditor::Clear() {
    // 何もしない
}

void SKKCandidateEditor::Output(SKKContextBuffer& buffer, bool active) const {
    if(active && !candidate_.IsEmpty()) {
        std::string str(candidate_.Variant());

        if(entry_.IsOkuriAri()) {
            str += entry_.OkuriString();
        }

        buffer.Compose("▼" + str);
        buffer.SetCandidate(candidate_);
    }

    buffer.SetEntry(entry_);
}

void SKKCandidateEditor::Commit(std::string& queue) {
    SKKBackEnd::theInstance().Register(entry_, candidate_);

    queue = candidate_.Variant();
    candidate_ = SKKCandidate();

    if(entry_.IsOkuriAri()) {
        queue += entry_.OkuriString();
    }
}

const SKKEntry SKKCandidateEditor::QueryEntry() const {
    return entry_;
}

void SKKCandidateEditor::SetCandidate(const SKKCandidate& candidate) {
    candidate_ = candidate;
}
