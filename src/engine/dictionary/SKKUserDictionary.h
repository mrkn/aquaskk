/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2002 phonohawk
  Copyright (C) 2005-2006 Tomotaka SUWA <t.suwa@mac.com>

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

#ifndef SKKUserDictionary_h
#define SKKUserDictionary_h

#include "SKKBaseDictionary.h"
#include "SKKDictionaryFile.h"
#include <vector>
#include <ctime>

class SKKUserDictionary: public SKKBaseDictionary {
    std::string path_;
    int idle_count_;
    std::time_t lastupdate_;
    SKKDictionaryFile file_;
    bool privateMode_;

    std::string fetch(const std::string& query, SKKDictionaryEntryContainer& container);
    void remove(const std::string& index, const std::string& kanji, SKKDictionaryEntryContainer& container);
    void save(bool force = false);

public:
    SKKUserDictionary();

    virtual ~SKKUserDictionary();

    virtual void Initialize(const std::string& path);

    virtual void FindOkuriAri(const std::string& entry, SKKCandidateSuite& result);
    virtual void FindOkuriNasi(const std::string& entry, SKKCandidateSuite& result);
    virtual std::string FindEntry(const std::string& candidate);

    virtual bool FindCompletions(const std::string& entry,
                                 std::vector<std::string>& result,
                                 unsigned minimumCompletionLength = 0);

    void RegisterOkuriAri(const std::string& query, const std::string& kana, const SKKCandidate& candidate);
    void RegisterOkuriNasi(const std::string& query, const SKKCandidate& candidate);

    void RemoveOkuriAri(const std::string& query, const SKKCandidate& candidate);
    void RemoveOkuriNasi(const std::string& query, const SKKCandidate& candidate);

    void SetPrivateMode(bool flag);
};

#endif
