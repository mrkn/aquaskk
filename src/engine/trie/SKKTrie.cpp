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

#include <iostream>
#include "SKKTrie.h"

SKKTrie::SKKTrie() : leaf_(false) {}

SKKTrie::SKKTrie(const std::string& hirakana,
		 const std::string& katakana,
		 const std::string& jisx0201kana,
		 const std::string& next)
    : leaf_(true), hirakana_(hirakana), katakana_(katakana), jisx0201kana_(jisx0201kana), next_(next) {}

void SKKTrie::Clear() {
    children_.clear();
}

void SKKTrie::Add(const std::string& str, const SKKTrie& node, unsigned depth) {
    // 末端か？
    if(depth == str.size() - 1) {
	children_[str[depth]] = node;
    } else {
	children_[str[depth]].Add(str, node, depth + 1); // 再帰追加
    }
}

const SKKTrie* SKKTrie::Traverse(const std::string& str, int& state, unsigned depth) {
    // [1] データ不足(ex. "k" や "ch" など)
    if(depth == str.size()) {
	state = 0;
	return 0;
    }

    // 一致？
    if(children_.find(str[depth]) != children_.end()) {
	SKKTrie* node = &children_[str[depth]];

	// 末端でないなら再帰検索
	if(!node->children_.empty()) {
	    return node->Traverse(str, state, depth + 1);
	}

	// [2] 完全一致
	state = depth + 1;
	return node;
    }

    // [3] 部分一致(ex. "kb" や "chm" など)
    if(0 < depth) {
	state = depth;

	// 節かつ葉でもある「n」のような場合には、一致として扱う
	if(leaf_) {
	    return this;
	}

	return 0;
    }

    // [4] 最初の一文字が木に存在しない
    state = -1;
    return 0;
}

const std::string& SKKTrie::KanaString(SKKInputMode mode) const {
    static std::string nothing;

    switch(mode) {
    case HirakanaInputMode:
	return hirakana_;

    case KatakanaInputMode:
	return katakana_;

    case Jisx0201KanaInputMode:
	return jisx0201kana_;

    default:
	std::cerr << "SKKTrie::KanaString(): invalid mode [" << mode << "]" << std::endl;
	return nothing;
    }
}

const std::string& SKKTrie::NextState() const {
    return next_;
}
