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
#include <fstream>
#include <sstream>
#include "jconv.h"
#include "SKKRomanKanaConverter.h"

// ======================================================================
// ユーティリティ
// ======================================================================
void unescape_string(std::string& str) {
    static struct {
	std::string from;
	const char* to;
    } escape[] = {
	{ "&comma;", "," },
	{ "&space;", " " },
	{ "&sharp;", "#" },
	{ "",      0x00 },
    };
	    
    for(int i = 0; escape[i].to != 0x00; ++ i) {
	std::string& target = escape[i].from;
	for(unsigned pos = str.find(target); pos != std::string::npos; pos = str.find(target)) {
	    str.replace(pos, target.length(), escape[i].to);
	}
    }
}

// ======================================================================
// SKKRomanKanaConverter インタフェース
// ======================================================================
SKKRomanKanaConverter::SKKRomanKanaConverter() {}

SKKRomanKanaConverter& SKKRomanKanaConverter::theInstance() {
    static SKKRomanKanaConverter obj;
    return obj;
}

void SKKRomanKanaConverter::Initialize(const std::string& path) {
    std::ifstream ifs(path.c_str());
    std::string str;

    if(!ifs) {
	std::cerr << "SKKRomanKanaConverter::Initialize(): can't open file [" << path << "]" << std::endl;
	return;
    }

    // 初期化しておく
    root_.Clear();

    while(std::getline(ifs, str)) {
	if(str.empty() || str[0] == '#') continue;

	// EUC-JP → UTF-8 変換
	std::string utf8;
	jconv::convert_eucj_to_utf8(str, utf8);

	// 全ての ',' を空白に置換して分解する
	std::replace(utf8.begin(), utf8.end(), ',', ' ');
	std::istringstream buf(utf8);

	// 変換ルールを読む
	std::string roman, hirakana, katakana, jisx0201kana, next;
	if(buf >> roman >> hirakana >> katakana >> jisx0201kana) {
	    // オプションの次状態も読む
	    buf >> next;

	    // エスケープ文字を元に戻す
	    unescape_string(roman);
	    unescape_string(hirakana);
	    unescape_string(katakana);
	    unescape_string(jisx0201kana);
	    unescape_string(next);

	    // ルール木に追加
	    root_.Add(roman, SKKTrie(hirakana, katakana, jisx0201kana, next));
	} else {
	    // 不正な形式
	    std::cerr << "SKKRomanKanaConverter::Initialize(): invalid rule [" << utf8 << "]" << std::endl;
	}
    }
}

bool SKKRomanKanaConverter::Execute(SKKInputMode mode, const std::string& in, std::string& out, std::string& next) {
    bool converted = false;
    std::string str(in);

    out.clear();
    next.clear();

    while(!str.empty()) {
	int state;
	const SKKTrie* node = root_.Traverse(str, state);

	// 変換できた？
	if(node) {
	    out += node->KanaString(mode);
	    next = node->NextState();
	    converted = true;
	} else {
	    converted = false;
	}

	// 部分的に一致しているがデータ不足のためこれ以上処理できない
	if(!state) {
	    next = str;
	    return false;
	}

	// 最初の一文字が木に存在しない場合、出力にコピーして次の文字を調べる
	if(state < 0) {
	    out += str[0];
	    state = 1;
	}
	
	// 調べた部分を削り取って次の文字を調べる
	str = str.substr(state);
    }

    return converted;
}
