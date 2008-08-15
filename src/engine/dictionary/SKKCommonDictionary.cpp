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

#include "SKKCommonDictionary.h"
#include <sys/stat.h>

// 非同期辞書ロードクラス
class SKKCommonDictionaryLoader : public SKKDictionaryLoader {
    std::string path_;
    std::time_t lastupdate_;
    bool first_;

    virtual bool run() {
	SKKDictionaryFile tmp;

	if(updated() && tmp.Load(path_)) {
	    tmp.Sort();
	    NotifyObserver(tmp);
	} else {
            if(first_) {
                first_ = false;
                NotifyObserver(tmp);
            }
        }

	return true;
    }

    bool updated() {
	struct stat st;

	if(stat(path_.c_str(), &st) == 0 && lastupdate_ < st.st_mtime) {
	    lastupdate_ = st.st_mtime;
	    return true;
	}

	return false;
    }

public:
    SKKCommonDictionaryLoader(const std::string& location) : path_(location), lastupdate_(0), first_(true) {}
};

// ======================================================================

SKKCommonDictionary::SKKCommonDictionary() : loader_(0) {}

void SKKCommonDictionary::Initialize(const std::string& path) {
    if(loader_.get()) return;

    loader_ = std::auto_ptr<SKKCommonDictionaryLoader>(new SKKCommonDictionaryLoader(path));
    keeper_.Initialize(loader_.get(), 60, 1);
}

std::string SKKCommonDictionary::FindOkuriAri(const std::string& query) {
    return keeper_.FindOkuriAri(query);
}

std::string SKKCommonDictionary::FindOkuriNasi(const std::string& query) {
    return keeper_.FindOkuriNasi(query);
}

bool SKKCommonDictionary::FindCompletions(const std::string& entry, std::vector<std::string>& result) {
    return keeper_.FindCompletions(entry, result);
}

// ファクトリメソッドの登録
#include "SKKDictionaryFactory.h"
static bool initialize = SKKRegisterFactoryMethod<SKKCommonDictionary>(SKKCommonDictionaryType);
