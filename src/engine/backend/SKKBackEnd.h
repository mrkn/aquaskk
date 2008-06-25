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

#ifndef	SKKBackEnd_h
#define SKKBackEnd_h

#include <string>
#include <vector>
#include "SKKCandidateSuite.h"
#include "SKKDictionaryKey.h"
#include "SKKDictionaryCache.h"
#include "SKKEntry.h"

class SKKBaseDictionary;
class SKKUserDictionary;

class SKKBackEnd {
    std::auto_ptr<SKKUserDictionary> userdict_;
    std::vector<SKKBaseDictionary*> dicts_;
    SKKDictionaryKeyContainer actives_;
    SKKDictionaryCache cache_;
    bool useNumericConversion_;
    bool enableExtendedCompletion_;

    SKKBackEnd();
    SKKBackEnd(const SKKBackEnd&);
    SKKBackEnd& operator=(const SKKBackEnd&);

    bool findOkuriAri(const SKKEntry& entry, SKKCandidateSuite& result);
    bool findOkuriNasi(const SKKEntry& entry, SKKCandidateSuite& result);

public:
    static SKKBackEnd& theInstance();

    void Initialize(const std::string& userdict_path, const SKKDictionaryKeyContainer& keys);

    // 補完
    bool Complete(const std::string& key, std::vector<std::string>& result);

    // 検索
    bool Find(const SKKEntry& entry, SKKCandidateSuite& result);

    // 登録
    void Register(const SKKEntry& entry, const SKKCandidate& candidate);

    // 削除
    void Remove(const SKKEntry& entry, const SKKCandidate& candidate);

    // オプション：数値変換
    void UseNumericConversion(bool flag);

    // オプション：拡張補完
    void EnableExtendedCompletion(bool flag);
};

#endif
