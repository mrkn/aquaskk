/*

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

#include "SKKDictionaryKeeper.h"
#include "SKKCandidateParser.h"
#include "jconv.h"
#include "utf8util.h"

namespace {
    // 逆引き用ファンクタ
    class NotInclude {
        std::string candidate_;

    public:
        NotInclude(const std::string& candidate) : candidate_(candidate) {}

        bool operator()(const SKKDictionaryEntry& entry) const {
            return entry.second.find(candidate_) == std::string::npos;
        }
    };
}

SKKDictionaryKeeper::SKKDictionaryKeeper() : timer_(0), loaded_(false) {}

void SKKDictionaryKeeper::Initialize(SKKDictionaryLoader* loader, int interval, int timeout) {
    if(timer_.get()) return;

    timeout_ = timeout;

    loader->Connect(this);

    timer_ = std::auto_ptr<pthread::timer>(new pthread::timer(loader, interval));
}

std::string SKKDictionaryKeeper::FindOkuriAri(const std::string& query) {
    return fetch(query, file_.OkuriAri());
}

std::string SKKDictionaryKeeper::FindOkuriNasi(const std::string& query) {
    return fetch(query, file_.OkuriNasi());
}

std::string SKKDictionaryKeeper::FindEntry(const std::string& candidate) {
    pthread::lock scope(condition_);

    if(!ready()) return "";

    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    SKKDictionaryEntryContainer entries;
    SKKCandidateParser parser;

    std::remove_copy_if(container.begin(), container.end(),
                        std::back_inserter(entries), NotInclude("/" + jconv::eucj_from_utf8(candidate)));

    for(unsigned i = 0; i < entries.size(); ++ i) {
        parser.Parse(jconv::utf8_from_eucj(entries[i].second));
        const SKKCandidateContainer& suite = parser.Candidates();

        if(std::find(suite.begin(), suite.end(), candidate) != suite.end()) {
            return jconv::utf8_from_eucj(entries[i].first);
        }
    }

    return "";
}

bool SKKDictionaryKeeper::FindCompletions(const std::string& entry,
                                          std::vector<std::string>& result,
                                          int minimumCompletionLength) {
    pthread::lock scope(condition_);

    if(!ready()) return false;

    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    std::string query = jconv::eucj_from_utf8(entry);
    bool lengthCheckNeeded = utf8::length(query) < minimumCompletionLength;

    for(SKKDictionaryEntryIterator iter = container.begin(); iter != container.end(); ++ iter) {
        if(iter->first.compare(0, query.length(), query) != 0) continue;

        std::string completion = jconv::utf8_from_eucj(iter->first);

        if(lengthCheckNeeded) {
            if(utf8::length(completion) <= minimumCompletionLength) continue;
        }

        result.push_back(completion);
    }

    return !result.empty();
}

// ------------------------------------------------------------

void SKKDictionaryKeeper::SKKDictionaryLoaderUpdate(const SKKDictionaryFile& file) {
    pthread::lock scope(condition_);

    file_ = file;

    loaded_ = true;

    condition_.signal();
}

std::string SKKDictionaryKeeper::fetch(const std::string& query, SKKDictionaryEntryContainer& container) {
    pthread::lock scope(condition_);

    if(!ready()) return "";

    std::string index = jconv::eucj_from_utf8(query);

    if(!std::binary_search(container.begin(), container.end(), index, SKKDictionaryEntryCompare())) {
	return "";
    }

    SKKDictionaryEntryIterator iter = std::lower_bound(container.begin(), container.end(),
						       index, SKKDictionaryEntryCompare());

    return jconv::utf8_from_eucj(iter->second);
}

bool SKKDictionaryKeeper::ready() {
    // 辞書のロードが完了するまで待つ
    if(!loaded_) {
        if(!condition_.wait(timeout_)) return false;
    }

    return true;
}
