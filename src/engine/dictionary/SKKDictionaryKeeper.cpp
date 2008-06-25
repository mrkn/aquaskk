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
#include "jconv.h"

class CompareFunctor {
    int length_;

    bool compare(const std::string& lhs, const std::string& rhs) const {
	return 0 < rhs.compare(0, length_, lhs, 0, length_);
    }

public:
    CompareFunctor(int length) : length_(length) {}

    bool operator()(const SKKDictionaryEntry& lhs, const SKKDictionaryEntry& rhs) const {
	return compare(lhs.first, rhs.first);
    }

    bool operator()(const SKKDictionaryEntry& lhs, const std::string& rhs) const {
	return compare(lhs.first, rhs);
    }

    bool operator()(const std::string& lhs, const SKKDictionaryEntry& rhs) const {
	return compare(lhs, rhs.first);
    }
};

SKKDictionaryKeeper::SKKDictionaryKeeper() : timer_(0) {}

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

bool SKKDictionaryKeeper::FindCompletions(const std::string& entry, std::vector<std::string>& result) {
    pthread::lock scope(condition_);

    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    if(container.empty()) {
        if(!condition_.wait(timeout_)) return false;
    }

    typedef std::pair<SKKDictionaryEntryIterator, SKKDictionaryEntryIterator> EntryRange;

    std::string index;
    jconv::convert_utf8_to_eucj(entry, index);

    EntryRange range = std::equal_range(container.begin(), container.end(), index, CompareFunctor(index.size()));

    for(SKKDictionaryEntryIterator iter = range.first; iter != range.second; ++ iter) {
        std::string tmp;
        jconv::convert_eucj_to_utf8(iter->first, tmp);
        result.push_back(tmp);
    }

    return range.first != range.second;
}

void SKKDictionaryKeeper::SKKDictionaryLoaderUpdate(const SKKDictionaryFile& file) {
    pthread::lock scope(condition_);

    file_ = file;

    condition_.signal();
}

std::string SKKDictionaryKeeper::fetch(const std::string& query, SKKDictionaryEntryContainer& container) {
    pthread::lock scope(condition_);

    if(container.empty()) {
	if(!condition_.wait(timeout_)) return "";
    }

    std::string index;
    std::string result;

    jconv::convert_utf8_to_eucj(query, index);

    if(!std::binary_search(container.begin(), container.end(), index, SKKDictionaryEntryCompare())) {
	return result;
    }

    SKKDictionaryEntryIterator iter = std::lower_bound(container.begin(), container.end(),
						       index, SKKDictionaryEntryCompare());

    jconv::convert_eucj_to_utf8(iter->second, result);

    return result;
}
