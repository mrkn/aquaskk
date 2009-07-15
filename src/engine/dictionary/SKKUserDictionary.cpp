/*

  MacOS X implementation of the SKK input method.

  Copyright (C) 2002 phonohawk
  Copyright (C) 2005-2008 Tomotaka SUWA <t.suwa@mac.com>

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
#include <cerrno>
#include "SKKUserDictionary.h"
#include "SKKCandidateSuite.h"
#include "utf8util.h"

namespace {
    static const int MAX_IDLE_COUNT = 20;
    static const int MAX_SAVE_INTERVAL = 60 * 5;

    // SKKDictionaryEntry と文字列を比較するファンクタ
    class CompareUserDictionaryEntry: public std::unary_function<SKKDictionaryEntry, bool> {
        const std::string str_;

    public:
        CompareUserDictionaryEntry(const std::string& str) : str_(str) {}

        bool operator()(const SKKDictionaryEntry& entry) const {
            return entry.first == str_;
        }
    };

    // 逆引き用ファンクタ(SKKDictionaryKeeper と重複)
    class NotInclude {
        std::string candidate_;

    public:
        NotInclude(const std::string& candidate) : candidate_(candidate) {}

        bool operator()(const SKKDictionaryEntry& entry) const {
            return entry.second.find(candidate_) == std::string::npos;
        }
    };

    template <typename T>
    void update(const std::string& index, const T& obj, SKKDictionaryEntryContainer& container) {
        SKKCandidateSuite suite;
        SKKDictionaryEntryIterator iter = std::find_if(container.begin(), container.end(),
                                                       CompareUserDictionaryEntry(index));

        if(iter != container.end()) {
            suite.Parse(iter->second);
            container.erase(iter);
        }

        suite.Update(obj);

        container.push_front(SKKDictionaryEntry(index, suite.ToString()));
    }
}

SKKUserDictionary::SKKUserDictionary() : privateMode_(false) {}

SKKUserDictionary::~SKKUserDictionary() {
    // 強制保存
    if(!path_.empty()) save(true);
}

void SKKUserDictionary::Initialize(const std::string& path) {
    if(!path_.empty()) {
        if(path_ == path) {
            return;
        }

        save(true);
    }

    path_ = path;
    idle_count_ = 0;
    lastupdate_ = std::time(0);

    if(!file_.Load(path)) {
	std::cerr << "SKKUserDictionary: can't load file: " << path << std::endl;
    }
}

void SKKUserDictionary::FindOkuriAri(const std::string& entry, SKKCandidateSuite& result) {
    SKKCandidateSuite suite(fetch(entry, file_.OkuriAri()));

    result.Add(suite);
}

void SKKUserDictionary::FindOkuriNasi(const std::string& entry, SKKCandidateSuite& result) {
    SKKCandidateSuite suite(fetch(entry, file_.OkuriNasi()));

    SKKCandidateContainer& candidates = suite.Candidates();

    std::for_each(candidates.begin(), candidates.end(), std::mem_fun_ref(&SKKCandidate::Decode));

    result.Add(suite);
}

std::string SKKUserDictionary::FindEntry(const std::string& candidate) {
    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    SKKDictionaryEntryContainer entries;
    SKKCandidateParser parser;

    std::remove_copy_if(container.begin(), container.end(),
                        std::back_inserter(entries), NotInclude("/" + candidate));

    for(unsigned i = 0; i < entries.size(); ++ i) {
        parser.Parse(entries[i].second);
        const SKKCandidateContainer& suite = parser.Candidates();

        if(std::find(suite.begin(), suite.end(), candidate) != suite.end()) {
            return entries[i].first;
        }
    }

    return "";
}

bool SKKUserDictionary::FindCompletions(const std::string& query,
                                        std::vector<std::string>& result,
                                        int minimumCompletionLength) {
    SKKDictionaryEntryContainer& container = file_.OkuriNasi();

    bool lengthCheckNeeded = utf8::length(query) < minimumCompletionLength;

    for(SKKDictionaryEntryIterator iter = container.begin(); iter != container.end(); ++ iter) {
        if(iter->first.compare(0, query.length(), query) != 0) continue;

        if(lengthCheckNeeded) {
            if(utf8::length(iter->first) <= minimumCompletionLength) continue;
        }

        result.push_back(iter->first);
    }

    return !result.empty();
}

void SKKUserDictionary::RegisterOkuriAri(const std::string& index, const std::string& okuri, const std::string& kanji) {
    SKKOkuriHint hint;

    hint.first = okuri;
    hint.second.push_back(kanji);

    update(index, hint, file_.OkuriAri());
    save();
}

void SKKUserDictionary::RegisterOkuriNasi(const std::string& index, const std::string& kanji) {
    update(index, SKKCandidate(kanji), file_.OkuriNasi());
    save();
}

void SKKUserDictionary::RemoveOkuriAri(const std::string& index, const std::string& kanji) {
    remove(index, kanji, file_.OkuriAri());
    save();
}

void SKKUserDictionary::RemoveOkuriNasi(const std::string& index, const std::string& kanji) {
    remove(index, kanji, file_.OkuriNasi());
    save();
}

void SKKUserDictionary::SetPrivateMode(bool flag) {
    if(privateMode_ != flag) {
        if(flag) {
            save(true);
        } else {
            file_.Load(path_);
        }

        privateMode_ = flag;
    }
}

// ======================================================================
// private method
// ======================================================================

std::string SKKUserDictionary::fetch(const std::string& query, SKKDictionaryEntryContainer& container) {
    SKKDictionaryEntryIterator iter = std::find_if(container.begin(), container.end(),
						   CompareUserDictionaryEntry(query));

    if(iter == container.end()) {
	return std::string();
    }

    return iter->second;
}

void SKKUserDictionary::remove(const std::string& index, const std::string& kanji,
			       SKKDictionaryEntryContainer& container) {
    SKKDictionaryEntryIterator iter = std::find_if(container.begin(), container.end(),
						   CompareUserDictionaryEntry(index));
    if(iter == container.end()) return;

    SKKCandidateSuite suite;

    suite.Parse(iter->second);
    suite.Remove(kanji);

    if(suite.IsEmpty()) {
	container.erase(iter);
    } else {
	iter->second = suite.ToString();
    }
}

void SKKUserDictionary::save(bool force) {
    if(privateMode_) return;

    if(!force && ++ idle_count_ < MAX_IDLE_COUNT && std::time(0) - lastupdate_ < MAX_SAVE_INTERVAL) {
	return;
    }

    idle_count_ = 0;
    lastupdate_ = std::time(0);

    std::string tmp_path = path_ + ".tmp";
    if(!file_.Save(tmp_path)) {
	std::cout << "SKKUserDictionary: can't save: " << tmp_path << std::endl;
	return;
    }

    if(rename(tmp_path.c_str(), path_.c_str()) < 0) {
	std::cout << "SKKUserDictionary: rename() failed[" << std::strerror(errno) << "]" << std::endl;
    } else {
	std::cout << "SKKUserDictionary: saved" << std::endl;
    }
}
