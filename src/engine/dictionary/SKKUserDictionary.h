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

#include <vector>
#include "SKKBaseDictionary.h"
#include "SKKDictionaryFile.h"

class SKKUserDictionary: public SKKBaseDictionary {
    std::string path_;
    int idle_count_;
    std::time_t lastupdate_;
    SKKDictionaryFile file_;

    std::string fetch(const std::string& query, SKKDictionaryEntryContainer& container);
    void remove(const std::string& index, const std::string& kanji, SKKDictionaryEntryContainer& container);
    void save(bool force = false);

public:
    virtual ~SKKUserDictionary();

    virtual void Initialize(const std::string& path);

    virtual std::string FindOkuriAri(const std::string& query);
    virtual std::string FindOkuriNasi(const std::string& query);

    virtual bool FindCompletions(const std::string& entry, std::vector<std::string>& result);

    void RegisterOkuriAri(const std::string& query, const std::string& kana, const std::string& candidate);
    void RegisterOkuriNasi(const std::string& query, const std::string& candidate);

    void RemoveOkuriAri(const std::string& query, const std::string& candidate);
    void RemoveOkuriNasi(const std::string& query, const std::string& candidate);
};

#endif
