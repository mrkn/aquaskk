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

namespace {
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
                    NotifyObserver(tmp);
                }
            }

            first_ = false;

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
        SKKCommonDictionaryLoader(const std::string& location)
            : path_(location), lastupdate_(0), first_(true) {}
    };
}

// ======================================================================

SKKCommonDictionary::SKKCommonDictionary() : loader_(0) {}

void SKKCommonDictionary::Initialize(const std::string& path) {
    if(loader_.get()) return;

    loader_ = std::auto_ptr<SKKCommonDictionaryLoader>(new SKKCommonDictionaryLoader(path));
    keeper_.Initialize(loader_.get(), 60, 1);
}

void SKKCommonDictionary::FindOkuriAri(const std::string& query, SKKCandidateSuite& result) {
    SKKCandidateSuite suite(keeper_.FindOkuriAri(query));

    result.Add(suite);
}

void SKKCommonDictionary::FindOkuriNasi(const std::string& query, SKKCandidateSuite& result) {
    SKKCandidateSuite suite(keeper_.FindOkuriNasi(query));

    result.Add(suite);
}

std::string SKKCommonDictionary::FindEntry(const std::string& candidate) {
    return keeper_.FindEntry(candidate);
}

bool SKKCommonDictionary::FindCompletions(const std::string& entry,
                                          std::vector<std::string>& result,
                                          int minimumCompletionLength) {
    return keeper_.FindCompletions(entry, result, minimumCompletionLength);
}

// ファクトリメソッドの登録
#include "SKKDictionaryFactory.h"
static bool initialize = SKKRegisterFactoryMethod<SKKCommonDictionary>(SKKCommonDictionaryType);
