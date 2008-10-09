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

#ifndef SKKAutoUpdateDictionary_h
#define SKKAutoUpdateDictionary_h

#include "SKKBaseDictionary.h"
#include "SKKDictionaryKeeper.h"

class SKKAutoUpdateDictionary : public SKKBaseDictionary {
    std::auto_ptr<SKKDictionaryLoader> loader_;
    SKKDictionaryKeeper keeper_;

public:
    SKKAutoUpdateDictionary();

    //
    // 引数の形式は "host:port url path" とする。":port" は省略化。
    //
    // 例)
    //
    // Initialize("openlab.ring.gr.jp /skk/skk/dic/SKK-JISYO.L /path/to/the/SKK-JISYO.L");
    //
    virtual void Initialize(const std::string& location);

    virtual std::string FindOkuriAri(const std::string& entry);
    virtual std::string FindOkuriNasi(const std::string& entry);
    virtual std::string FindEntry(const std::string& candidate);

    virtual bool FindCompletions(const std::string& entry, std::vector<std::string>& result);
};

#endif
