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

#ifndef SKKRomanKanaConverter_h
#define SKKRomanKanaConverter_h

#include <string>
#include "SKKTrie.h"
#include "SKKInputMode.h"

class SKKRomanKanaConverter {
    SKKTrie root_;

    SKKRomanKanaConverter();
    SKKRomanKanaConverter(const SKKRomanKanaConverter&);

public:
    static SKKRomanKanaConverter& theInstance();

    void Initialize(const std::string& path);

    // ローマ字かな変換
    //
    // 引数：
    //	in=ローマ字文字列
    //	out=結果文字列(変換されなくても設定されることはある)
    //	next=次状態文字列
    //
    // 戻り値：
    //	true=変換された、false=変換されなかった
    //
    bool Execute(SKKInputMode mode, const std::string& in, std::string& out, std::string& next);
};

#endif
