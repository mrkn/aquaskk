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

#ifndef SKKContextBuffer_h
#define SKKContextBuffer_h

#include <string>
#include "SKKEntry.h"
#include "SKKCandidate.h"

class SKKFrontEnd;

class SKKContextBuffer {
    std::string fixed_;
    std::string composing_;
    std::string completion_;
    int cursor_;
    int completion_cursor_;

    SKKEntry entry_;
    SKKCandidate candidate_;

public:
    SKKContextBuffer();

    void Fix(const std::string& str);
    void Compose(const std::string& str, int cursor = 0);
    void Compose(const std::string& str, const std::string& completion, int cursor = 0);
    bool IsComposing() const;

    void Output(SKKFrontEnd* frontend);

    void SetEntry(const SKKEntry& entry);
    void SetCandidate(const SKKCandidate& candidate);

    void Clear();

    const SKKEntry& Entry() const;
    const SKKCandidate& Candidate() const;

    void Dump() const;
};

#endif
