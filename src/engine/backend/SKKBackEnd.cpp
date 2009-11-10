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

#include <iostream>
#include <set>
#include "SKKBackEnd.h"
#include "SKKUserDictionary.h"
#include "SKKCandidateSuite.h"
#include "SKKNumericConverter.h"

namespace {
    typedef void (SKKBaseDictionary::*FindMethod)(const std::string&, SKKCandidateSuite&);

    // 検索用ファンクタ
    class Invoke {
        FindMethod method_;
        std::string entry_;
        SKKCandidateSuite* result_;

    public:
        Invoke(FindMethod method, const SKKEntry& entry, SKKCandidateSuite& result)
            : method_(method), entry_(entry.EntryString()), result_(&result) {}

        void operator()(SKKBaseDictionary* dict) const {
            (dict->*method_)(entry_, *result_);
        }
    };


    // 数値変換用ファンクタ
    class NumericConversion {
        SKKNumericConverter* converter_;

    public:
        NumericConversion(SKKNumericConverter& converter)
            : converter_(&converter) {}

        SKKCandidate& operator()(SKKCandidate& candidate) const {
            converter_->Apply(candidate);
            return candidate;
        }
    };
}

SKKBackEnd::SKKBackEnd()
    : userdict_(new SKKUserDictionary())
    , useNumericConversion_(false)
    , enableExtendedCompletion_(false)
    , minimumCompletionLength_(0)
{}

SKKBackEnd& SKKBackEnd::theInstance() {
    static SKKBackEnd obj;
    return obj;
}

void SKKBackEnd::Initialize(const std::string& userdict_path, const SKKDictionaryKeyContainer& keys) {
    userdict_->Initialize(userdict_path);

    // 不要な辞書を破棄する
    for(unsigned i = 0; i < actives_.size(); ++ i) {
	if(std::find(keys.begin(), keys.end(), actives_[i]) == keys.end()) {
	    cache_.Clear(actives_[i]);
	}
    }

    // 辞書を初期化する
    dicts_.clear();
    dicts_.push_back(userdict_.get());
    for(unsigned i = 0; i < keys.size(); ++ i) {
	dicts_.push_back(cache_.Get(keys[i]));
    }

    actives_ = keys;
}

bool SKKBackEnd::Complete(const std::string& key, std::vector<std::string>& result, unsigned limit) {
    // ユーザー辞書を優先
    result.clear();
    userdict_->FindCompletions(key, result, minimumCompletionLength_);

    if(key.empty() || !enableExtendedCompletion_) {
        return !result.empty();
    }

    // 重複チェック用のセット
    std::set<std::string> check(result.begin(), result.end());
    check.insert(key);

    // 各辞書に対して補完を試みる
    for(unsigned i = 0; i < dicts_.size(); ++ i) {
        // 充分な補完候補が見つかった？
        if(limit != 0 && limit <= result.size()) break;

        std::vector<std::string> tmp;

        if(!dicts_[i]->FindCompletions(key, tmp, minimumCompletionLength_)) continue;

        for(unsigned j = 0; j < tmp.size(); ++ j) {
            if(check.find(tmp[j]) == check.end()) {
                check.insert(tmp[j]);
                result.push_back(tmp[j]);
            }
        }
    }

    return !result.empty();
}

bool SKKBackEnd::Find(const SKKEntry& entry, SKKCandidateSuite& result) {
    FindMethod method = &SKKBaseDictionary::FindOkuriNasi;
    bool okuriAri = entry.IsOkuriAri();

    result.Clear();

    if(okuriAri) {
        method = &SKKBaseDictionary::FindOkuriAri;
    }

    std::for_each(dicts_.begin(), dicts_.end(), Invoke(method, entry, result));

    if(okuriAri) {
        std::string okuri(entry.OkuriString());
        if(!okuri.empty()) {
            SKKOkuriHintContainer& hints = result.Hints();
            std::partition(hints.begin(), hints.end(), CompareOkuriHint(okuri));
        }
    } else {
        SKKNumericConverter converter;
        if(useNumericConversion_ && converter.Setup(entry.EntryString())) {
            SKKCandidateSuite suite;
            std::for_each(dicts_.begin(), dicts_.end(),
                          Invoke(method, converter.NormalizedKey(), suite));

            SKKCandidateContainer& cands = suite.Candidates();
            std::transform(cands.begin(), cands.end(),
                           std::back_inserter(result.Candidates()),
                           NumericConversion(converter));
        }

        result.Remove(SKKCandidate(converter.OriginalKey()));
    }

    return !result.IsEmpty();
}

std::string SKKBackEnd::ReverseLookup(const std::string& candidate) {
    if(candidate.empty()) return "";

    for(unsigned i = 0; i < dicts_.size(); ++ i) {
        std::string entry(dicts_[i]->FindEntry(candidate));

        if(!entry.empty()) {
            return entry;
        }
    }

    return "";
}

void SKKBackEnd::Register(const SKKEntry& entry, const SKKCandidate& candidate) {
    if(entry.EntryString().empty() ||
       (entry.IsOkuriAri() && (entry.OkuriString().empty() || candidate.IsEmpty()))) {
	std::cerr << "SKKBackEnd: Invalid registration received" << std::endl;
	return;
    }

    if(candidate.AvoidStudy()) {
        return;
    }
    
    if(entry.IsOkuriAri()) {
	userdict_->RegisterOkuriAri(entry.EntryString(), entry.OkuriString(), candidate);
    } else {
        userdict_->RegisterOkuriNasi(normalizedKey(entry), candidate);
    }
}

void SKKBackEnd::Remove(const SKKEntry& entry, const SKKCandidate& candidate) {
    if(entry.EntryString().empty()) {
	std::cerr << "SKKBackEnd: Invalid removal received" << std::endl;
	return;
    }

    if(entry.IsOkuriAri()) {
	userdict_->RemoveOkuriAri(entry.EntryString(), candidate);
    } else {
	userdict_->RemoveOkuriNasi(normalizedKey(entry), candidate);
    }
}

void SKKBackEnd::UseNumericConversion(bool flag) {
    useNumericConversion_ = flag;
}

void SKKBackEnd::EnableExtendedCompletion(bool flag) {
    enableExtendedCompletion_ = flag;
}

void SKKBackEnd::EnablePrivateMode(bool flag) {
    userdict_->SetPrivateMode(flag);
}

void SKKBackEnd::SetMinimumCompletionLength(int length) {
    minimumCompletionLength_ = length;
}

// ----------------------------------------------------------------------

std::string SKKBackEnd::normalizedKey(const SKKEntry& entry) {
    std::string key(entry.EntryString());
    SKKNumericConverter converter;

    if(useNumericConversion_ && converter.Setup(key)) {
        // 単語登録と削除時には、数値だけの見出し語を正規化しない
        if(converter.NormalizedKey() != "#") {
            return converter.NormalizedKey();
        }
    }

    return key;
}
