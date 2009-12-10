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

#ifndef SKKDictionaryKeeper_h
#define SKKDictionaryKeeper_h

#include "SKKDictionaryFile.h"
#include "SKKDictionaryLoader.h"
#include "pthreadutil.h"
#include <vector>
#include <memory>

class SKKDictionaryKeeper : public SKKDictionaryLoaderObserver {
    std::auto_ptr<pthread::timer> timer_;
    pthread::condition condition_;
    SKKDictionaryFile file_;
    bool loaded_;
    int timeout_;

    virtual void SKKDictionaryLoaderUpdate(const SKKDictionaryFile& file);

    std::string fetch(const std::string& query, SKKDictionaryEntryContainer& container);
    bool ready();

public:
    SKKDictionaryKeeper();

    void Initialize(SKKDictionaryLoader* loader, int interval, int timeout);

    // 通常の検索
    std::string FindOkuriAri(const std::string& query);
    std::string FindOkuriNasi(const std::string& query);

    // 逆引き
    std::string FindEntry(const std::string& candidate);

    // 見出し語補完
    bool FindCompletions(const std::string& entry,
                         std::vector<std::string>& result,
                         unsigned minimumCompletionLength = 0);
};

#endif
