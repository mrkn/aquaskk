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

SKKBackEnd::SKKBackEnd()
    : userdict_(new SKKUserDictionary())
    , useNumericConversion_(false)
    , enableExtendedCompletion_(false)
    , enablePrivateMode_(false)
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

bool SKKBackEnd::Complete(const std::string& key, std::vector<std::string>& result) {
    // ユーザー辞書を優先
    result.clear();
    userdict_->FindCompletions(key, result);

    if(key.empty() || !enableExtendedCompletion_) {
        return !result.empty();
    }

    // 重複チェック用のセット
    std::set<std::string> check(result.begin(), result.end());
    check.insert(key);

    // 各辞書に対して補完を試みる
    for(unsigned i = 0; i < dicts_.size(); ++ i) {
        std::vector<std::string> tmp;

        if(!dicts_[i]->FindCompletions(key, tmp)) continue;

        for(unsigned j = 0; j < tmp.size(); ++ j) {
            if(check.find(tmp[j]) == check.end()) {
                check.insert(tmp[j]);
                result.push_back(tmp[j]);
            }
        }
    }

    return !result.empty();
}

// 数値変換用ファンクタ
class NumericConversionFunctor {
    SKKNumericConverter* converter_;

public:
    NumericConversionFunctor(SKKNumericConverter* converter) : converter_(converter) {}

    SKKCandidate& operator()(SKKCandidate& candidate) const {
        converter_->Apply(candidate);
        return candidate;
    }
};

bool SKKBackEnd::Find(const SKKEntry& entry, SKKCandidateSuite& result) {
    if(entry.IsOkuriAri()) {
	return findOkuriAri(entry, result);
    } else {
        SKKCandidateSuite tmp;
        SKKNumericConverter converter;

	findOkuriNasi(entry, result);

        if(useNumericConversion_ && converter.Setup(entry.EntryString())) {
            findOkuriNasi(converter.NormalizedKey(), tmp);

            SKKCandidateContainer& container = tmp.Candidates();

            std::transform(container.begin(), container.end(), container.begin(),
                           NumericConversionFunctor(&converter));
        }

        // 数値変換の結果に関わらず、重複した候補を除外する
        mergeCandidates(converter.OriginalKey(), result, tmp);

        return !result.IsEmpty();
    }
}

void SKKBackEnd::Register(const SKKEntry& entry, const SKKCandidate& candidate) {
    if(enablePrivateMode_) return;

    if(entry.EntryString().empty() ||
       (entry.IsOkuriAri() && (entry.OkuriString().empty() || candidate.IsEmpty()))) {
	std::cerr << "SKKBackEnd: Invalid registration received" << std::endl;
	return;
    }
    
    if(entry.IsOkuriAri()) {
	userdict_->RegisterOkuriAri(entry.EntryString(), entry.OkuriString(), candidate.ToString());
    } else {
        SKKNumericConverter converter;
        std::string key(entry.EntryString());

        if(useNumericConversion_ && converter.Setup(key)) {
            key = converter.NormalizedKey();
        }

        // 登録時は常にエンコードする
        SKKCandidate tmp(candidate);
        tmp.Encode();
        userdict_->RegisterOkuriNasi(key, tmp.ToString());
    }
}

void SKKBackEnd::Remove(const SKKEntry& entry, const SKKCandidate& candidate) {
    if(entry.EntryString().empty() || candidate.IsEmpty()) {
	std::cerr << "SKKBackEnd: Invalid removal received" << std::endl;
	return;
    }

    if(entry.IsOkuriAri()) {
	userdict_->RemoveOkuriAri(entry.EntryString(), candidate.ToString());
    } else {
        SKKNumericConverter converter;
        std::string key(entry.EntryString());

        if(useNumericConversion_ && converter.Setup(key)) {
            key = converter.NormalizedKey();
        }

	userdict_->RemoveOkuriNasi(key, SKKCandidate::Encode(candidate.ToString()));
    }
}

void SKKBackEnd::UseNumericConversion(bool flag) {
    useNumericConversion_ = flag;
}

void SKKBackEnd::EnableExtendedCompletion(bool flag) {
    enableExtendedCompletion_ = flag;
}

void SKKBackEnd::EnablePrivateMode(bool flag) {
    enablePrivateMode_ = flag;
}

// ----------------------------------------------------------------------

bool SKKBackEnd::findOkuriAri(const SKKEntry& entry, SKKCandidateSuite& result) {
    SKKCandidateSuite normal;
    SKKCandidateParser parser;
    std::string key(entry.EntryString());
    std::string okuri(entry.OkuriString());

    result.Clear();

    for(unsigned i = 0; i < dicts_.size(); ++ i) {
	parser.Parse(dicts_[i]->FindOkuriAri(key));

	if(okuri.empty()) {
	    result.Add(parser.Hints());
	} else {
	    SKKOkuriHintContainer hints = parser.Hints();
	    SKKOkuriHintIterator iter = std::find_if(hints.begin(), hints.end(), CompareOkuriHint(okuri));

	    if(iter != hints.end()) {
		result.Add(*iter);
		hints.erase(iter);
		normal.Add(hints);
	    }
	}

	normal.Add(parser.Candidates());
    }

    result.Add(normal);

    return !result.IsEmpty();
}

bool SKKBackEnd::findOkuriNasi(const SKKEntry& entry, SKKCandidateSuite& result) {
    SKKCandidateSuite tmp;
    std::string key(entry.EntryString());

    result.Clear();

    for(unsigned i = 0; i < dicts_.size(); ++ i) {
	tmp.Parse(dicts_[i]->FindOkuriNasi(key));

        // ユーザー辞書のエントリはデコードする
        if(dicts_[i] == userdict_.get()) {
            SKKCandidateContainer& candidates = tmp.Candidates();
            std::for_each(candidates.begin(), candidates.end(),
                          std::mem_fun_ref(&SKKCandidate::Decode));
        }

	result.Add(tmp);
    }

    return !result.IsEmpty();
}

typedef std::set<std::string> StringSet;

// マージ用述語
class ExistIn {
    StringSet* check_;

public:
    ExistIn(StringSet* check) : check_(check) {}

    bool operator()(const SKKCandidate& candidate) const {
        std::string word(candidate.Variant());
            
        if(check_->find(word) == check_->end()) {
            check_->insert(word);
            return false;       // 存在しない
        }

        return true;            // 存在する
    }
};

void SKKBackEnd::mergeCandidates(const std::string& key, SKKCandidateSuite& result1, SKKCandidateSuite& result2) {
    StringSet candidates;
    SKKCandidateSuite result;
    SKKCandidateContainer* in;
    SKKCandidateContainer& out = result.Candidates();

    // 数値変換で使用した見出し語も重複チェックに使う
    candidates.insert(key);

    in = &result1.Candidates();
    std::remove_copy_if(in->begin(), in->end(), std::back_inserter(out), ExistIn(&candidates));

    in = &result2.Candidates();
    std::remove_copy_if(in->begin(), in->end(), std::back_inserter(out), ExistIn(&candidates));

    result1 = result;
}
