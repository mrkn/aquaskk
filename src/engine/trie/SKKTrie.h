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

#include <string>
#include <map>
#include "SKKInputMode.h"

#ifndef SKKTrie_h
#define SKKTrie_h

// kana-rule-list の木表現
class SKKTrie {
    bool leaf_;
    std::string hirakana_;
    std::string katakana_;
    std::string jisx0201kana_;
    std::string next_;

    std::map<char, SKKTrie> children_;

public:
    SKKTrie();
    SKKTrie(const std::string& hirakana, const std::string& katakana,
	    const std::string& jisx0201kana, const std::string& next);

    // 初期化
    void Clear();

    // 再帰的ノード追加
    //
    // 引数：
    //	str=変換文字列
    //	node=追加ノード
    //	depth=探索の深度(再帰専用)
    //
    // 例：
    // 	node.Add("gya", SKKTrie("ぎゃ", "ギャ", "ｷﾞｬ", ""));
    //
    void Add(const std::string& str, const SKKTrie& node, unsigned depth = 0);

    // 再帰的ノード検索
    //
    // 引数：
    //	str=変換文字列
    //	state=状態
    //		-1	最初の一文字が木に存在しないことを示す
    //		0	部分一致していることを示す(変換も削り取りもしない)
    //		1 以上	変換文字列から削り取る文字数を示す(std::string::substr の引数)
    //	depth=探索の深度(再帰専用)
    //
    // 戻り値：
    //	変換結果ノードへのポインタ、変換できなかった場合は 0
    //
    const SKKTrie* Traverse(const std::string& str, int& state, unsigned depth = 0);

    // かな文字列取得
    const std::string& KanaString(SKKInputMode mode) const;

    // 次状態文字列取得
    const std::string& NextState() const;
};

#endif
