/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2007 Tomotaka SUWA <t.suwa@mac.com>

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

#ifndef	SKKCandidateSuite_h
#define SKKCandidateSuite_h

#include "SKKCandidateParser.h"

// 送りヒントの比較は「かな」部分のみ
class CompareOkuriHint {
    std::string kana_;

public:
    CompareOkuriHint(const std::string& okuri) : kana_(okuri) {}
    CompareOkuriHint(const SKKOkuriHint& hint) : kana_(hint.first) {}

    bool operator()(const SKKOkuriHint& hint) const {
	return kana_ == hint.first;
    }
};

// ======================================================================
// 見出し語に対応する一組の候補
// ======================================================================
class SKKCandidateSuite {
    SKKCandidateContainer candidates_;
    SKKOkuriHintContainer hints_;
    SKKCandidateParser parser_;

    // 重複チェック用ファンクタ
    class contains {
	SKKCandidateContainer& container_;

    public:
	contains(SKKCandidateContainer& src) : container_(src) {}

	bool operator()(const SKKCandidate& item) const {
	    return !item.IsEmpty() &&
		std::find(container_.begin(), container_.end(), item) != container_.end();
	}
    };

    void add(SKKCandidateContainer& container, const SKKCandidateContainer& src) {
	std::remove_copy_if(src.begin(), src.end(), std::back_inserter(container), contains(container));
    }

    void update(SKKCandidateContainer& container, const SKKCandidate& candidate) {
	remove(container, candidate);
	container.push_front(candidate);
    }

    void remove(SKKCandidateContainer& container, const SKKCandidate& candidate) {
	container.erase(std::remove(container.begin(), container.end(), candidate), container.end());
    }

    void flatten(const SKKCandidateContainer& container, std::string& result, bool exclude_avoid_study) const {
	for(unsigned i = 0; i < container.size(); ++ i) {
            if(exclude_avoid_study && container[i].AvoidStudy()) {
                continue;
            }
	    result += "/";
	    result += container[i].ToString();
	}
    }

public:
    SKKCandidateSuite() {}

    SKKCandidateSuite(const std::string& line) {
        Parse(line);
    }

    void Parse(const std::string& str) {
	parser_.Parse(str);

	candidates_ = parser_.Candidates();
	hints_ = parser_.Hints();
    }

    void Clear() {
	candidates_.clear();
	hints_.clear();
    }

    bool IsEmpty() const {
	return candidates_.empty();
    }

    void Add(const SKKCandidate& candidate) {
	SKKCandidateContainer tmp;

	tmp.push_back(candidate);
	add(candidates_, tmp);
    }

    void Add(const SKKOkuriHint& hint) {
	SKKOkuriHintIterator iter = std::find_if(hints_.begin(), hints_.end(), CompareOkuriHint(hint));

	if(iter != hints_.end()) {
	    add(iter->second, hint.second);
	} else {
	    hints_.push_back(hint);
	}
    }

    void Add(const SKKCandidateContainer& candidates) {
	add(candidates_, candidates);
    }

    void Add(const SKKOkuriHintContainer& hints) {
	for(unsigned i = 0; i < hints.size(); ++ i) {
	    Add(hints[i]);
	}
    }

    void Add(const SKKCandidateSuite& suite) {
	add(candidates_, suite.candidates_);
	Add(suite.hints_);
    }

    void Update(const SKKCandidate& candidate) {
	update(candidates_, candidate);
    }

    void Update(const SKKOkuriHint& hint) {
	if(hint.second.empty()) return;

	update(candidates_, hint.second[0]);

	SKKOkuriHintIterator iter = std::find_if(hints_.begin(), hints_.end(), CompareOkuriHint(hint));
	if(iter != hints_.end()) {
	    update(iter->second, hint.second[0]);
	} else {
	    hints_.push_front(hint);
	}
    }

    void Remove(const SKKCandidate& candidate) {
	remove(candidates_, candidate);

	SKKOkuriHintIterator iter = hints_.begin();
	while(iter != hints_.end()) {
	    remove(iter->second, candidate);

	    if(iter->second.empty()) {
		iter = hints_.erase(iter);
	    } else {
		++ iter;
	    }
	}
    }

    SKKCandidateContainer& Candidates() {
	return candidates_;
    }

    SKKOkuriHintContainer& Hints() {
        return hints_;
    }

    const std::string ToString(bool exclude_avoid_study = false) const {
	std::string str;

	flatten(candidates_, str, exclude_avoid_study);

	if(!hints_.empty()) {
	    for(unsigned i = 0; i < hints_.size(); ++ i) {
		str += "/[";
		str += hints_[i].first;

		flatten(hints_[i].second, str, false);

		str += "/]";
	    }
	}

	if(!str.empty()) {
	    str += "/";
	}

	return str;
    }
};

#endif
