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

#include "SKKEntry.h"

SKKEntry::SKKEntry() {}

SKKEntry::SKKEntry(const std::string& entry, const std::string& okuri)
    : entry_(entry), okuri_(okuri) {}

void SKKEntry::SetOkuri(const std::string& head, const std::string& okuri) {
    prompt_ = entry_ + "*" + okuri;
    entry_ += head;
    okuri_ = okuri;
}

const std::string& SKKEntry::EntryString() const {
    return entry_;
}

const std::string& SKKEntry::OkuriString() const {
    return okuri_;
}

const std::string& SKKEntry::OkuriPrompt() const {
    return prompt_;
}

bool SKKEntry::IsEmpty() const {
    return entry_.empty();
}

bool SKKEntry::IsOkuriAri() const {
    return !okuri_.empty();
}
